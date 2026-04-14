// 2026 S1C COMP2017 Week 7 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Demonstration for `signal()` and signal handlers.


#include <signal.h>
#include <unistd.h>

#define MSG "I was interrupted!\n"

void interrupt_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, MSG, sizeof(MSG) - 1);
}

int main(void) {
    signal(SIGINT, interrupt_handler);

    while (1) {
        pause();
    }
}
