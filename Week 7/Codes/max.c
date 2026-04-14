// 2026 S1C COMP2017 Week 7 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A `max` function through function pointer returns a pointer to the maximum
// element which defined by different calls:
// 1. int_value()
// 2. string_length_value()
// 3. struct_count_value()


#include <stdio.h>
#include <string.h>

struct item {
    char *name;
    int count;
};

int int_value(const void *a) {
    return *(const int *)a;
}

int string_length_value(const void *a) {
    return (int)strlen(*(const char * const *)a);
}

int struct_count_value(const void *a) {
    return ((const struct item *)a)->count;
}

void *max(void *base, size_t n, size_t size, int (*value)(const void *)) {
    if (base == NULL || n == 0 || value == NULL) {
        return NULL;
    }

    char *bytes = (char *)base;

    void *max_elem = bytes;
    int max_val = value(max_elem);

    for (size_t i = 1; i < n; i++) {
        void *current = bytes + i * size;
        int current_val = value(current);

        if (current_val > max_val) {
            max_val = current_val;
            max_elem = current;
        }
    }

    return max_elem;
}

int main(void) {
    int nums[] = {10, 55, 23, 9, 42};
    int *max_num = max(nums, 5, sizeof(int), int_value);
    printf("%d\n", *max_num);

    char *words[] = {"apple", "hippopotamus", "cat", "banana"};
    char **max_word = max(words, 4, sizeof(char *), string_length_value);
    printf("%s\n", *max_word);

    struct item items[] = {
        {"apple", 1},
        {"banana", 500},
        {"cat", 15}
    };
    struct item *max_item = max(items, 3, sizeof(struct item), struct_count_value);
    printf("%s %d\n", max_item->name, max_item->count);

    return 0;
}
