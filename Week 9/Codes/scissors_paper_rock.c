// 2026 S1C COMP2017 Week 9 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A program that the parent server process watches two children player processes
// to play scissors-paper-rock through IPC. Each move is randomly made.


#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define LINE_SIZE 128
#define NUM_ROUNDS 3
#define ROUND_DELAY_SECONDS 5

static int send_line(FILE *stream, const char *line) {
    if (fprintf(stream, "%s\n", line) < 0) {
        return -1;
    }

    if (fflush(stream) == EOF) {
        return -1;
    }

    return 0;
}

static int read_line(FILE *stream, char *buf, size_t size) {
    if (fgets(buf, size, stream) == NULL) {
        return 0;
    }

    size_t len = strlen(buf);

    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }

    return 1;
}

static const char *random_move(void) {
    static const char *moves[] = {
        "rock",
        "paper",
        "scissors"
    };

    return moves[rand() % 3];
}

static const char *result_for(const char *mine, const char *opponent) {
    if (strcmp(mine, opponent) == 0) {
        return "We drew";
    }

    if (
        (strcmp(mine, "rock") == 0 && strcmp(opponent, "scissors") == 0) ||
        (strcmp(mine, "scissors") == 0 && strcmp(opponent, "paper") == 0) ||
        (strcmp(mine, "paper") == 0 && strcmp(opponent, "rock") == 0)
    ) {
        return "I won, nice";
    }

    return "I lost, sad days";
}

static int run_player(const char *name, int read_fd, int write_fd) {
    FILE *in = fdopen(read_fd, "r");

    if (in == NULL) {
        perror("fdopen read");
        close(read_fd);
        close(write_fd);
        return 1;
    }

    FILE *out = fdopen(write_fd, "w");

    if (out == NULL) {
        perror("fdopen write");
        fclose(in);
        close(write_fd);
        return 1;
    }

    srand((unsigned) time(NULL) ^ (unsigned) getpid());

    printf("%s: ONLINE\n", name);
    fflush(stdout);

    if (send_line(out, "ONLINE") == -1) {
        perror("send ONLINE");
        fclose(in);
        fclose(out);
        return 1;
    }

    char command[LINE_SIZE];

    while (read_line(in, command, sizeof(command))) {
        if (strcmp(command, "STOP") == 0) {
            break;
        }

        if (strcmp(command, "GO") != 0) {
            fprintf(stderr, "%s: unexpected command: %s\n", name, command);
            break;
        }

        const char *mine = random_move();

        if (send_line(out, mine) == -1) {
            perror("send move");
            fclose(in);
            fclose(out);
            return 1;
        }

        char opponent[LINE_SIZE];

        if (!read_line(in, opponent, sizeof(opponent))) {
            fprintf(stderr, "%s: server closed pipe\n", name);
            break;
        }

        printf(
            "%s: I played %s, opponent played %s. %s\n",
            name,
            mine,
            opponent,
            result_for(mine, opponent)
        );

        fflush(stdout);
    }

    fclose(in);
    fclose(out);

    return 0;
}

static int run_server(
    int to_p1_fd,
    int from_p1_fd,
    int to_p2_fd,
    int from_p2_fd
) {
    FILE *to_p1 = fdopen(to_p1_fd, "w");

    if (to_p1 == NULL) {
        perror("fdopen to_p1");
        close(to_p1_fd);
        close(from_p1_fd);
        close(to_p2_fd);
        close(from_p2_fd);
        return 1;
    }

    FILE *from_p1 = fdopen(from_p1_fd, "r");

    if (from_p1 == NULL) {
        perror("fdopen from_p1");
        fclose(to_p1);
        close(from_p1_fd);
        close(to_p2_fd);
        close(from_p2_fd);
        return 1;
    }

    FILE *to_p2 = fdopen(to_p2_fd, "w");

    if (to_p2 == NULL) {
        perror("fdopen to_p2");
        fclose(to_p1);
        fclose(from_p1);
        close(to_p2_fd);
        close(from_p2_fd);
        return 1;
    }

    FILE *from_p2 = fdopen(from_p2_fd, "r");

    if (from_p2 == NULL) {
        perror("fdopen from_p2");
        fclose(to_p1);
        fclose(from_p1);
        fclose(to_p2);
        close(from_p2_fd);
        return 1;
    }

    char status1[LINE_SIZE];
    char status2[LINE_SIZE];

    if (
        !read_line(from_p1, status1, sizeof(status1)) ||
        !read_line(from_p2, status2, sizeof(status2))
    ) {
        fprintf(stderr, "server: players did not come online\n");

        fclose(to_p1);
        fclose(from_p1);
        fclose(to_p2);
        fclose(from_p2);

        return 1;
    }

    if (strcmp(status1, "ONLINE") != 0 || strcmp(status2, "ONLINE") != 0) {
        fprintf(stderr, "server: unexpected online status\n");

        fclose(to_p1);
        fclose(from_p1);
        fclose(to_p2);
        fclose(from_p2);

        return 1;
    }

    for (int round = 1; round <= NUM_ROUNDS; round++) {
        sleep(ROUND_DELAY_SECONDS);

        if (send_line(to_p1, "GO") == -1 || send_line(to_p2, "GO") == -1) {
            perror("server send GO");
            break;
        }

        char p1_move[LINE_SIZE];
        char p2_move[LINE_SIZE];

        if (
            !read_line(from_p1, p1_move, sizeof(p1_move)) ||
            !read_line(from_p2, p2_move, sizeof(p2_move))
        ) {
            fprintf(stderr, "server: failed to read player moves\n");
            break;
        }

        if (
            send_line(to_p1, p2_move) == -1 ||
            send_line(to_p2, p1_move) == -1
        ) {
            perror("server send opponent move");
            break;
        }
    }

    send_line(to_p1, "STOP");
    send_line(to_p2, "STOP");

    fclose(to_p1);
    fclose(from_p1);
    fclose(to_p2);
    fclose(from_p2);

    return 0;
}

int main(void) {
    int to_p1[2];
    int from_p1[2];
    int to_p2[2];
    int from_p2[2];

    if (pipe(to_p1) == -1) {
        perror("pipe to_p1");
        return 1;
    }

    if (pipe(from_p1) == -1) {
        perror("pipe from_p1");
        return 1;
    }

    if (pipe(to_p2) == -1) {
        perror("pipe to_p2");
        return 1;
    }

    if (pipe(from_p2) == -1) {
        perror("pipe from_p2");
        return 1;
    }

    pid_t p1 = fork();

    if (p1 == -1) {
        perror("fork player 1");
        return 1;
    }

    if (p1 == 0) {
        close(to_p1[1]);
        close(from_p1[0]);

        close(to_p2[0]);
        close(to_p2[1]);
        close(from_p2[0]);
        close(from_p2[1]);

        exit(run_player("Player 1", to_p1[0], from_p1[1]));
    }

    pid_t p2 = fork();

    if (p2 == -1) {
        perror("fork player 2");
        return 1;
    }

    if (p2 == 0) {
        close(to_p2[1]);
        close(from_p2[0]);

        close(to_p1[0]);
        close(to_p1[1]);
        close(from_p1[0]);
        close(from_p1[1]);

        exit(run_player("Player 2", to_p2[0], from_p2[1]));
    }

    close(to_p1[0]);
    close(from_p1[1]);

    close(to_p2[0]);
    close(from_p2[1]);

    int server_status = run_server(
        to_p1[1],
        from_p1[0],
        to_p2[1],
        from_p2[0]
    );

    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);

    return server_status;
}
