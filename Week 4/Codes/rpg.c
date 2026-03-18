// 2026 S1C COMP2017 Week 4 Tutorial A Role Playing Game
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// https://edstem.org/au/courses/31567/lessons/99434/slides/682655


#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint8_t inventory;   // 8 item bits
    uint8_t state;       // low 4 bits = status, high 4 bits = health
} Character;

enum Item {
    ITEM_SWORD = 0,
    ITEM_SHIELD,
    ITEM_POTION,
    ITEM_KEY,
    ITEM_PICK_AXE,
    ITEM_WAND,
    ITEM_SANDWICH,
    ITEM_JUICE,
    ITEM_COUNT
};

enum Status {
    STATUS_POISONED = 0,
    STATUS_BURNING,
    STATUS_FROZEN,
    STATUS_CURSED,
    STATUS_COUNT
};

#define HEALTH_SHIFT 4
#define HEALTH_MASK  0xF0

const char *item_names[ITEM_COUNT] = {
    "sword", "shield", "potion", "key",
    "pick axe", "wand", "sandwich", "juice"
};

const char *status_names[STATUS_COUNT] = {
    "poisoned", "burning", "frozen", "cursed"
};

int set_health(Character *ch, unsigned health) {
    if (health > 15) {
        return -1;
    }

    ch->state &= (uint8_t)~HEALTH_MASK;                 // clear old health bits
    ch->state |= (uint8_t)(health << HEALTH_SHIFT);     // store new health
    return 0;
}

unsigned get_health(const Character *ch) {
    return (ch->state & HEALTH_MASK) >> HEALTH_SHIFT;
}

int has_item(const Character *ch, int item) {
    if (item < 0 || item >= ITEM_COUNT) {
        return 0;
    }

    return (ch->inventory & (1u << item)) != 0;
}

int add_item(Character *ch, int item) {
    if (item < 0 || item >= ITEM_COUNT) {
        return -1;
    }

    ch->inventory |= (uint8_t)(1u << item);
    return 0;
}

int apply_status(Character *ch, int status) {
    if (status < 0 || status >= STATUS_COUNT) {
        return -1;
    }

    ch->state |= (uint8_t)(1u << status);
    return 0;
}

int use_potion(Character *ch) {
    unsigned health = get_health(ch);

    if (!has_item(ch, ITEM_POTION)) {
        return -1;   // no potion available
    }

    if (health == 0) {
        return -1;   // would underflow
    }

    set_health(ch, health - 1);

    // inventory only stores presence/absence, so using the potion removes it
    ch->inventory &= (uint8_t)~(1u << ITEM_POTION);

    return 0;
}

void print_status(const Character *ch) {
    int i;
    int first;

    printf("Health: %u/15\n", get_health(ch));

    printf("Inventory: ");
    first = 1;
    for (i = 0; i < ITEM_COUNT; i++) {
        if (has_item(ch, i)) {
            if (!first) {
                printf(", ");
            }
            printf("%s", item_names[i]);
            first = 0;
        }
    }
    if (first) {
        printf("empty");
    }
    printf("\n");

    printf("Potion available: %s\n",
           has_item(ch, ITEM_POTION) ? "yes" : "no");

    printf("Status effects: ");
    first = 1;
    for (i = 0; i < STATUS_COUNT; i++) {
        if (ch->state & (1u << i)) {
            if (!first) {
                printf(", ");
            }
            printf("%s", status_names[i]);
            first = 0;
        }
    }
    if (first) {
        printf("none");
    }
    printf("\n");
}

int main(void) {
    Character hero = {0, 0};

    printf("sizeof(Character) = %zu bytes\n\n", sizeof(Character));

    set_health(&hero, 9);

    add_item(&hero, ITEM_SWORD);
    add_item(&hero, ITEM_SHIELD);
    add_item(&hero, ITEM_POTION);
    add_item(&hero, ITEM_KEY);

    apply_status(&hero, STATUS_POISONED);
    apply_status(&hero, STATUS_CURSED);

    print_status(&hero);

    printf("\nUsing potion...\n");
    if (use_potion(&hero) != 0) {
        printf("Could not use potion.\n");
    }

    print_status(&hero);

    if (add_item(&hero, 9) != 0) {
        printf("\nInvalid item id 9\n");
    }

    return 0;
}
