// 2026 S1C COMP2017 Week 6 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Dynamic Array Data Structure.


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 4

typedef struct dyn_array dyn_array;

struct dyn_array {
    int size;
    int capacity;
    int *contents;
};

static int dyn_array_resize(dyn_array *dyn, int new_capacity);
dyn_array* dyn_array_init(void);
void dyn_array_add(dyn_array *dyn, int value);
void dyn_array_delete(dyn_array *dyn, int index);
int dyn_array_get(dyn_array *dyn, int index);
void dyn_array_set(dyn_array *dyn, int index, int value);
void dyn_array_free(dyn_array *dyn);
void dyn_array_print(dyn_array *dyn);


int main(void) {
    dyn_array *dyn = dyn_array_init();
    if (dyn == NULL) {
        return 1;
    }

    dyn_array_add(dyn, 10);
    dyn_array_add(dyn, 20);
    dyn_array_add(dyn, 30);
    dyn_array_add(dyn, 40);
    dyn_array_add(dyn, 50);   // forces resize

    dyn_array_print(dyn);

    printf("Element at index 2 = %d\n", dyn_array_get(dyn, 2));

    dyn_array_set(dyn, 1, 99);
    dyn_array_print(dyn);

    dyn_array_delete(dyn, 3);
    dyn_array_print(dyn);

    dyn_array_free(dyn);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

static int dyn_array_resize(dyn_array *dyn, int new_capacity) {
    int *new_contents = realloc(
        dyn->contents,
        (size_t)new_capacity * sizeof *new_contents
    );

    if (new_contents == NULL) {
        return 0;
    }

    dyn->contents = new_contents;
    dyn->capacity = new_capacity;
    return 1;
}

dyn_array* dyn_array_init(void) {
    dyn_array *dyn = malloc(sizeof *dyn);
    if (dyn == NULL) {
        return NULL;
    }

    dyn->size = 0;
    dyn->capacity = INITIAL_CAPACITY;
    dyn->contents = malloc((size_t)dyn->capacity * sizeof *dyn->contents);

    if (dyn->contents == NULL) {
        free(dyn);
        return NULL;
    }

    return dyn;
}

void dyn_array_add(dyn_array *dyn, int value) {
    if (dyn == NULL) {
        return;
    }

    if (dyn->size == dyn->capacity) {
        if (!dyn_array_resize(dyn, dyn->capacity * 2)) {
            return;
        }
    }

    dyn->contents[dyn->size] = value;
    dyn->size++;
}

void dyn_array_delete(dyn_array *dyn, int index) {
    if (dyn == NULL || index < 0 || index >= dyn->size) {
        return;
    }

    for (int i = index; i < dyn->size - 1; i++) {
        dyn->contents[i] = dyn->contents[i + 1];
    }

    dyn->size--;
}

int dyn_array_get(dyn_array *dyn, int index) {
    assert(dyn != NULL);
    assert(index >= 0 && index < dyn->size);

    return dyn->contents[index];
}

void dyn_array_set(dyn_array *dyn, int index, int value) {
    assert(dyn != NULL);
    assert(index >= 0 && index < dyn->size);

    dyn->contents[index] = value;
}

void dyn_array_free(dyn_array *dyn) {
    if (dyn == NULL) {
        return;
    }

    free(dyn->contents);
    free(dyn);
}

void dyn_array_print(dyn_array *dyn) {
    if (dyn == NULL) {
        printf("NULL\n");
        return;
    }

    printf("size = %d, capacity = %d, contents = [", dyn->size, dyn->capacity);
    for (int i = 0; i < dyn->size; i++) {
        printf("%d", dyn->contents[i]);
        if (i + 1 < dyn->size) {
            printf(", ");
        }
    }
    printf("]\n");
}
