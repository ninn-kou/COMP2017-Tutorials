// 2026 S1C COMP2017 Week 4 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Encode the run length of the same symbol and return the length of compression.


#include <stdio.h>

int encode_run(const char* sequence, unsigned len, char* buf, unsigned int buf_len) {
    unsigned i = 0;
    unsigned out = 0;

    if (buf_len == 0) {
        return -1;
    }

    while (i < len && sequence[i] != '\0') {
        unsigned count = 1;

        while (i + count < len &&
               sequence[i + count] != '\0' &&
               sequence[i + count] == sequence[i]) {
            count++;
        }

        if (out + 1 >= buf_len) {
            buf[0] = '\0';
            return -1;
        }

        buf[out] = '0' + count;
        out++;
        i += count;
    }

    buf[out] = '\0';
    return (int)out;
}

int main(void) {
    char encoded_run[128];
    const char* line_run = "1122333334423";

    encode_run(line_run, 14, encoded_run, 128);
    printf("%s\n", encoded_run); // 225211

    encode_run("778889", 6, encoded_run, 128);
    printf("%s\n", encoded_run); // 231

    encode_run("9999", 4, encoded_run, 128);
    printf("%s\n", encoded_run); // 4

    encode_run("", 0, encoded_run, 128);
    printf("%s\n", encoded_run); // ""

    return 0;
}
