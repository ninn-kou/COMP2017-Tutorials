// 2026 S1C COMP2017 Week 6 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// My `cat` command by using system calls as an extension of Week 5 Tutorial A exercise `cat`.
// Multiple files supported.


#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

static int write_all(int fd, const char *buf, size_t count) {
    while (count > 0) {
        ssize_t nwritten = write(fd, buf, count);

        if (nwritten < 0) {
            // [EINTR] Interrupted function.
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (nwritten == 0) {
            return -1;
        }

        buf += nwritten;
        count -= (size_t)nwritten;
    }

    return 0;
}

static void report_error(const char *name) {
    int saved_errno = errno;
    const char *msg = strerror(saved_errno);

    write_all(STDERR_FILENO, "cat: ", 5);
    write_all(STDERR_FILENO, name, strlen(name));
    write_all(STDERR_FILENO, ": ", 2);
    write_all(STDERR_FILENO, msg, strlen(msg));
    write_all(STDERR_FILENO, "\n", 1);
}

static int copy_fd(int input_fd) {
    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t nread = read(input_fd, buffer, sizeof buffer);

        if (nread == 0) {
            return 0;   // EOF
        }

        if (nread < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (write_all(STDOUT_FILENO, buffer, (size_t)nread) != 0) {
            return -1;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        if (copy_fd(STDIN_FILENO) != 0) {
            report_error("stdin");
            return 1;
        }
        return 0;
    }

    int status = 0;

    for (int i = 1; i < argc; i++) {
        // Refer to the manual of open(2).
        // https://man7.org/linux/man-pages/man2/open.2.html
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            report_error(argv[i]);
            status = 1;
            continue;
        }

        if (copy_fd(fd) != 0) {
            report_error(argv[i]);
            status = 1;
        }

        if (close(fd) < 0) {
            report_error(argv[i]);
            status = 1;
        }
    }

    return status;
}
