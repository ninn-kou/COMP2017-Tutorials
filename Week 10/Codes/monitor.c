// 2026 S1C COMP2017 Week 10 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A program that one parent process to monitor output from five child processes.


#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

#define N_CHILDREN 5
#define BUF_SIZE 256

static volatile sig_atomic_t got_sigusr1 = 0;

static void on_sigusr1(int sig) {
    (void)sig;
    got_sigusr1 = 1;
}

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void child_main(int i, int pipes[N_CHILDREN][2]) {
    for (int j = 0; j < i; j++) {
        close(pipes[j][0]);
    }

    close(pipes[i][0]);

    int devnull = open("/dev/null", O_RDONLY);
    if (devnull == -1) {
        perror("open /dev/null");
        _exit(1);
    }

    if (dup2(devnull, STDIN_FILENO) == -1) {
        perror("dup2 stdin");
        _exit(1);
    }
    close(devnull);

    if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
        perror("dup2 stdout");
        _exit(1);
    }

    if (dup2(pipes[i][1], STDERR_FILENO) == -1) {
        perror("dup2 stderr");
        _exit(1);
    }

    close(pipes[i][1]);

    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_sigusr1;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        _exit(1);
    }

    printf("child %d: hello from pid %ld\n", i, (long)getpid());
    fflush(stdout);

    while (!got_sigusr1) {
        pause();
    }

    _exit(0);
}

static void spawn_children(int pipes[N_CHILDREN][2],
                           pid_t pids[N_CHILDREN]) {
    for (int i = 0; i < N_CHILDREN; i++) {
        if (pipe(pipes[i]) == -1) {
            die("pipe");
        }

        pid_t pid = fork();

        if (pid == -1) {
            die("fork");
        }

        if (pid == 0) {
            child_main(i, pipes);
        }

        pids[i] = pid;
        close(pipes[i][1]);
    }
}

static void print_message(int child, const char *buf, ssize_t nread) {
    printf("parent read from child %d: ", child);
    fwrite(buf, 1, (size_t)nread, stdout);

    if (nread == 0 || buf[nread - 1] != '\n') {
        putchar('\n');
    }

    fflush(stdout);
}

static void signal_child_once(int child,
                              pid_t pids[N_CHILDREN],
                              int signalled[N_CHILDREN]) {
    if (!signalled[child]) {
        if (kill(pids[child], SIGUSR1) == -1) {
            perror("kill");
        }

        signalled[child] = 1;
    }
}

static int read_and_release(int child,
                            int fd,
                            pid_t pids[N_CHILDREN],
                            int signalled[N_CHILDREN]) {
    char buf[BUF_SIZE];
    ssize_t nread;

    do {
        nread = read(fd, buf, sizeof buf);
    } while (nread == -1 && errno == EINTR);

    if (nread > 0) {
        print_message(child, buf, nread);
        signal_child_once(child, pids, signalled);
        return 1;
    }

    if (nread == 0) {
        printf("child %d closed pipe without output\n", child);
        return 1;
    }

    perror("read");
    signal_child_once(child, pids, signalled);
    return 1;
}

static void monitor_with_poll(int pipes[N_CHILDREN][2],
                              pid_t pids[N_CHILDREN]) {
    struct pollfd fds[N_CHILDREN];
    int signalled[N_CHILDREN] = {0};
    int active = N_CHILDREN;

    for (int i = 0; i < N_CHILDREN; i++) {
        fds[i].fd = pipes[i][0];
        fds[i].events = POLLIN | POLLHUP;
        fds[i].revents = 0;
    }

    while (active > 0) {
        int nready = poll(fds, N_CHILDREN, -1);

        if (nready == -1) {
            if (errno == EINTR) {
                continue;
            }

            die("poll");
        }

        for (int i = 0; i < N_CHILDREN; i++) {
            if (fds[i].fd == -1) {
                continue;
            }

            if (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {
                if (read_and_release(i, fds[i].fd, pids, signalled)) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    active--;
                }
            }
        }
    }
}

static void monitor_with_epoll(int pipes[N_CHILDREN][2],
                               pid_t pids[N_CHILDREN]) {
    int epfd = epoll_create1(0);

    if (epfd == -1) {
        die("epoll_create1");
    }

    for (int i = 0; i < N_CHILDREN; i++) {
        struct epoll_event ev;
        memset(&ev, 0, sizeof ev);

        ev.events = EPOLLIN | EPOLLHUP;
        ev.data.u32 = (unsigned)i;

        if (epoll_ctl(epfd, EPOLL_CTL_ADD, pipes[i][0], &ev) == -1) {
            die("epoll_ctl ADD");
        }
    }

    struct epoll_event events[N_CHILDREN];
    int signalled[N_CHILDREN] = {0};
    int active = N_CHILDREN;

    while (active > 0) {
        int nready = epoll_wait(epfd, events, N_CHILDREN, -1);

        if (nready == -1) {
            if (errno == EINTR) {
                continue;
            }

            die("epoll_wait");
        }

        for (int i = 0; i < nready; i++) {
            int child = (int)events[i].data.u32;
            int fd = pipes[child][0];

            if (fd == -1) {
                continue;
            }

            if (events[i].events & (EPOLLIN | EPOLLHUP | EPOLLERR)) {
                if (read_and_release(child, fd, pids, signalled)) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    pipes[child][0] = -1;
                    active--;
                }
            }
        }
    }

    close(epfd);
}

int main(int argc, char *argv[]) {
    if (argc != 2 ||
        (strcmp(argv[1], "poll") != 0 && strcmp(argv[1], "epoll") != 0)) {
        fprintf(stderr, "Usage: %s poll|epoll\n", argv[0]);
        return 1;
    }

    int pipes[N_CHILDREN][2];
    pid_t pids[N_CHILDREN];

    spawn_children(pipes, pids);

    if (strcmp(argv[1], "poll") == 0) {
        monitor_with_poll(pipes, pids);
    } else {
        monitor_with_epoll(pipes, pids);
    }

    for (int i = 0; i < N_CHILDREN; i++) {
        waitpid(pids[i], NULL, 0);
    }

    printf("process complete\n");
    return 0;
}
