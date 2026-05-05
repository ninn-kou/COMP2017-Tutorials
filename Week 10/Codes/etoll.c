// 2026 S1C COMP2017 Week 10 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
//
// A toll‑booth server that handles vehicle payments on N separate lanes. Assume
// that each line will handle M cars.
//
// Each lane is represented by a non-blocking file descriptor (e.g. a pipe,
// socket). “Cars” arrive at random intervals on any lane and send a small
// payment message. The booth must process each payment and log it, then
// immediately be ready for the next car—across all lanes—without busy‑waiting.


#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define NLANES 3
#define CARS_PER_LANE 4
#define MSG_SIZE 128

/* Print an error message and stop the program. */
static void die(const char *msg) {
    perror(msg);
    exit(1);
}

/* Make a file descriptor non-blocking. */
static void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL);

    if (flags == -1) {
        die("fcntl F_GETFL");
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        die("fcntl F_SETFL");
    }
}

/* This function runs in a child process.
   It pretends to be cars arriving on one lane. */
static void car_generator(int lane, int fd) {
    srand((unsigned)(time(NULL) ^ getpid()));

    /* Send several car payment messages. */
    for (int i = 0; i < CARS_PER_LANE; i++) {
        sleep((unsigned)(rand() % 3));   // wait 0, 1, or 2 seconds

        int amount = 5 + rand() % 20;

        char msg[MSG_SIZE];

        int len = snprintf(msg, sizeof msg,
                           "CAR-%d-%d:%d\n",
                           lane, i, amount);

        if (len < 0 || len >= MSG_SIZE) {
            _exit(1);
        }

        if (write(fd, msg, (size_t)len) == -1) {
            perror("child write");
            _exit(1);
        }
    }

    /* Tell the server this lane is finished. */
    if (write(fd, "DONE\n", 5) == -1) {
        perror("child write DONE");
    }

    close(fd);
    _exit(0);
}

/* Stop watching one lane and close it. */
static void close_lane(int epfd, int lane, int lane_fds[], int *active_lanes) {
    if (lane_fds[lane] == -1) {
        return;
    }

    epoll_ctl(epfd, EPOLL_CTL_DEL, lane_fds[lane], NULL);
    close(lane_fds[lane]);

    lane_fds[lane] = -1;
    (*active_lanes)--;
}

int main(void) {
    int lane_fds[NLANES];
    pid_t children[NLANES];

    /* Step 1: create each lane and fork one car generator for it. */
    for (int lane = 0; lane < NLANES; lane++) {
        int sv[2];

        /* socketpair gives us two connected endpoints:
           sv[0] will be used by the server.
           sv[1] will be used by the child car generator. */
        if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) == -1) {
            die("socketpair");
        }

        pid_t pid = fork();

        if (pid == -1) {
            die("fork");
        }

        if (pid == 0) {
            /* Child process */

            close(sv[0]);   // child does not use the server side

            /* Close old server fds inherited from earlier loop iterations. */
            for (int old = 0; old < lane; old++) {
                close(lane_fds[old]);
            }

            car_generator(lane, sv[1]);
        }

        /* Parent process */

        close(sv[1]);              // parent does not use the child side
        set_nonblocking(sv[0]);    // server reads should not block

        lane_fds[lane] = sv[0];
        children[lane] = pid;
    }

    /* Step 2: create the epoll object. */
    int epfd = epoll_create1(0);

    if (epfd == -1) {
        die("epoll_create1");
    }

    /* Step 3: register every lane with epoll. */
    for (int lane = 0; lane < NLANES; lane++) {
        struct epoll_event ev = {0};

        ev.events = EPOLLIN;          // wait until this lane has data
        ev.data.u32 = (unsigned)lane; // remember which lane this is

        /* This is level-triggered by default.
           We did not use EPOLLET, so this is not edge-triggered. */
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, lane_fds[lane], &ev) == -1) {
            die("epoll_ctl ADD");
        }
    }

    /* Step 4: main toll-booth loop. */
    int active_lanes = NLANES;
    struct epoll_event events[NLANES];

    while (active_lanes > 0) {
        /* Wait until at least one lane has a message.
           This blocks without busy-waiting. */
        int nready = epoll_wait(epfd, events, NLANES, -1);

        if (nready == -1) {
            if (errno == EINTR) {
                continue;
            }

            die("epoll_wait");
        }

        /* Handle all lanes that became ready. */
        for (int i = 0; i < nready; i++) {
            int lane = (int)events[i].data.u32;
            char buf[MSG_SIZE];

            if (lane_fds[lane] == -1) {
                continue;
            }

            /* Read one message from this lane.
               Because the fd is non-blocking, this will not freeze the server. */
            ssize_t nread = read(lane_fds[lane], buf, sizeof buf - 1);

            if (nread > 0) {
                buf[nread] = '\0';

                /* DONE means this car generator has finished. */
                if (strcmp(buf, "DONE\n") == 0) {
                    printf("lane %d finished\n", lane);
                    close_lane(epfd, lane, lane_fds, &active_lanes);
                } else {
                    printf("lane %d payment: %s", lane, buf);
                }
            } else if (nread == -1 &&
                       (errno == EAGAIN || errno == EWOULDBLOCK)) {
                /* epoll said ready, but no data is available now.
                   With non-blocking I/O, this is not fatal. */
                continue;
            } else if (nread == -1 && errno == EINTR) {
                /* Interrupted by a signal. Try again later. */
                continue;
            } else {
                /* Unexpected error or closed lane. */
                printf("lane %d closed unexpectedly\n", lane);
                close_lane(epfd, lane, lane_fds, &active_lanes);
            }
        }
    }

    /* Step 5: wait for all child processes to finish. */
    for (int lane = 0; lane < NLANES; lane++) {
        waitpid(children[lane], NULL, 0);
    }

    close(epfd);

    printf("process complete\n");
    return 0;
}
