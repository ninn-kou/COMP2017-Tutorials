// 2026 S1C COMP2017 Week 4 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Run-length Tuples Codes.


#include <stdio.h>
#include <string.h>

struct tuple {
    char symbol;
    int count;
};

int encode_run(const char *input, int len, struct tuple out[], int max_out) {
    if (input == NULL || out == NULL || len < 0 || max_out < 0) {
        return -1;
    }

    if (len == 0 || input[0] == '\0') {
        return 0;
    }

    int run_count = 0;
    char current = input[0];
    int count = 1;

    for (int i = 1; i < len && input[i] != '\0'; i++) {
        if (input[i] == current) {
            count++;
        } else {
            if (run_count >= max_out) {
                return -1;
            }

            out[run_count].symbol = current;
            out[run_count].count = count;
            run_count++;

            current = input[i];
            count = 1;
        }
    }

    if (run_count >= max_out) {
        return -1;
    }

    out[run_count].symbol = current;
    out[run_count].count = count;
    run_count++;

    return run_count;
}

int print_run(const struct tuple runs[], int run_count, char *buffer, int buffer_size) {
    if (runs == NULL || buffer == NULL || run_count < 0 || buffer_size <= 0) {
        return -1;
    }

    int used = 0;
    buffer[0] = '\0';

    for (int i = 0; i < run_count; i++) {
        int written = snprintf(
            buffer + used,
            (size_t)(buffer_size - used),
            (i == 0) ? "(%c,%d)" : " (%c,%d)",
            runs[i].symbol,
            runs[i].count
        );

        if (written < 0 || written >= buffer_size - used) {
            return -1;
        }

        used += written;
    }

    return used;
}

int decode_run(const struct tuple runs[], int run_count, char *output, int output_size) {
    if (runs == NULL || output == NULL || run_count < 0 || output_size <= 0) {
        return -1;
    }

    int pos = 0;

    for (int i = 0; i < run_count; i++) {
        if (runs[i].count < 0) {
            return -1;
        }

        for (int j = 0; j < runs[i].count; j++) {
            if (pos >= output_size - 1) {
                return -1;
            }
            output[pos++] = runs[i].symbol;
        }
    }

    output[pos] = '\0';
    return pos;
}

int main(void) {
    const char *line_run = "1122333334423";

    struct tuple runs[128];
    char encoded_run[128];
    char decoded_run[128];

    int run_count = encode_run(line_run, (int)strlen(line_run), runs, 128);
    if (run_count == -1) {
        printf("Encoding failed\n");
        return 1;
    }

    if (print_run(runs, run_count, encoded_run, 128) == -1) {
        printf("Printing failed\n");
        return 1;
    }

    if (decode_run(runs, run_count, decoded_run, 128) == -1) {
        printf("Decoding failed\n");
        return 1;
    }

    printf("original: %s\n", line_run);
    printf("encoded : %s\n", encoded_run);
    printf("decoded : %s\n", decoded_run);

    return 0;
}
