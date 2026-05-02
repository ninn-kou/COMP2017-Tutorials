// 2026 S1C COMP2017 Week 9 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A parent/child signal dialogue.


#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static void die(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

static void say(const char *speaker, const char *line) {
    printf("%s: %s\n", speaker, line);
    fflush(stdout);
}

static void send_turn(pid_t pid) {
    if (kill(pid, SIGINT) == -1) {
        die("kill");
    }
}

static void wait_for_turn(const sigset_t *turn_set) {
    int received_signal = 0;

    int err = sigwait(turn_set, &received_signal);
    if (err != 0) {
        errno = err;
        die("sigwait");
    }

    if (received_signal != SIGINT) {
        fprintf(stderr, "unexpected signal: %d\n", received_signal);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    sigset_t turn_set;

    if (sigemptyset(&turn_set) == -1) {
        die("sigemptyset");
    }

    if (sigaddset(&turn_set, SIGINT) == -1) {
        die("sigaddset");
    }

    /*
     * Block SIGINT before fork().
     *
     * This prevents a lost-signal race. If SIGINT arrives before sigwait(),
     * it becomes pending instead of being lost.
     */
    if (sigprocmask(SIG_BLOCK, &turn_set, NULL) == -1) {
        die("sigprocmask");
    }

    pid_t parent_pid = getpid();
    pid_t child_pid = fork();

    if (child_pid == -1) {
        die("fork");
    }

    if (child_pid == 0) {
        wait_for_turn(&turn_set);
        say("C", "What are you talking about?");
        send_turn(parent_pid);

        wait_for_turn(&turn_set);
        say("C", "Oh, that one. Of course, I always do my COMP2017 homework!");
        send_turn(parent_pid);

        return EXIT_SUCCESS;
    }

    say("P", "Have you done your homework?");
    send_turn(child_pid);

    wait_for_turn(&turn_set);
    say("P", "The homework that is due tomorrow, so you can pass COMP2017.");
    send_turn(child_pid);

    wait_for_turn(&turn_set);
    say("P", "You are a great child, I'm so proud of you.");

    int status = 0;
    if (waitpid(child_pid, &status, 0) == -1) {
        die("waitpid");
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "child did not exit cleanly\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
