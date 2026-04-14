## COMP2017 2026 S1 Week 6 Tutorial A

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 6 Tutorial A](#comp2017-2026-s1-week-6-tutorial-a)
  - [A.1 Doubly and Circular Linked List](#a1-doubly-and-circular-linked-list)
    - [A.1.1 Doubly Linked List](#a11-doubly-linked-list)
    - [A.1.2 Circular Linked List](#a12-circular-linked-list)
  - [A.2 Exercise: Circular Linked List](#a2-exercise-circular-linked-list)
    - [A.2.1 Node Structure](#a21-node-structure)
    - [A.2.2 `list_init()`](#a22-list_init)
    - [A.2.3 `list_add()`](#a23-list_add)
    - [A.2.4 `list_delete()`](#a24-list_delete)
      - [Case A: Deleting The Only Node](#case-a-deleting-the-only-node)
      - [Case B: Deleting a Node From a Larger List](#case-b-deleting-a-node-from-a-larger-list)
    - [A.2.5 `list_next()`](#a25-list_next)
    - [A.2.6 `list_print()`](#a26-list_print)
    - [A.2.7 `list_free()`](#a27-list_free)
  - [A.3 Exercise: Detect Cycle in Linked List](#a3-exercise-detect-cycle-in-linked-list)
  - [A.4 Exercise: Dynamic Array](#a4-exercise-dynamic-array)
    - [A.4.1 Define the Structure](#a41-define-the-structure)
    - [A.4.2 Pick a Default Capacity](#a42-pick-a-default-capacity)
    - [A.4.3 Resizing Helper](#a43-resizing-helper)
    - [A.4.4 `dyn_array_init()`](#a44-dyn_array_init)
    - [A.4.5 `dyn_array_add()`](#a45-dyn_array_add)
    - [A.4.6 `dyn_array_get()`](#a46-dyn_array_get)
    - [A.4.7 `dyn_array_set()`](#a47-dyn_array_set)
    - [A.4.8 `dyn_array_delete()`](#a48-dyn_array_delete)
    - [A.4.9 `dyn_array_free`](#a49-dyn_array_free)
    - [A.4.10 Optional Extension: Shrinking](#a410-optional-extension-shrinking)
  - [A.5 Void Pointers](#a5-void-pointers)
    - [A.5.1 `void *` in C](#a51-void--in-c)
    - [A.5.2 Why `malloc()` Returns `void *`](#a52-why-malloc-returns-void-)
    - [A.5.3 `const void *`](#a53-const-void-)
  - [A.6 Generic Lists](#a6-generic-lists)

---

### A.1 Doubly and Circular Linked List

#### A.1.1 Doubly Linked List

A doubly linked list is a linear data structure where each node contains a data field and two pointers, referencing both the next and previous nodes. This bidirectional structure allows efficient traversal in both directions and enables faster deletion of nodes compared to singly linked lists.

![Doubly Linked List](../assets/img/DoublyList.svg)

To insert a node, we have the illustration looks like:

![Doubly Linked List Insert Step 1](../assets/img/doubly_insert_1.svg)

![Doubly Linked List Insert Step 2](../assets/img/doubly_insert_2.svg)

![Doubly Linked List Insert Step 3](../assets/img/doubly_insert_3.svg)

And to remove a node, we have:

![Doubly Linked List Remove Step 1](../assets/img/doubly_remove_1.svg)

![Doubly Linked List Remove Step 2](../assets/img/doubly_remove_2.svg)

![Doubly Linked List Remove Step 3](../assets/img/doubly_remove_3.svg)

![Doubly Linked List Remove Step 4](../assets/img/doubly_remove_4.svg)

#### A.1.2 Circular Linked List

For circular linked list,
- empty list: `head == NULL`
- one-node list: `head->next == head` and `head->prev == head`
- multi-node list: `head->prev` is the tail, and `tail->next == head`

That means there is **no `NULL` at the end anymore**, so traversal must stop when we come back to `head`. We will implement it step-by-step in next section.

---

### A.2 Exercise: Circular Linked List

Because our has both `prev` and `next`, this is actually a **circular doubly linked list**.

The biggest change from the singly linked list is this:

- in the old list, the last node had `next == NULL`
- in the circular list, the last node's `next` points back to the head
- also, the head's `prev` points to the last node

So for a non-empty list:

```text
head -> 10 <-> 20 <-> 30 <-> 40
 ^                            |
 |____________________________|
```

> [!IMPORTANT]
> Refer to [`circular.c`](./Codes/circular.c) for the code used in this section.

#### A.2.1 Node Structure

In the original list, a node only had `next`.

Now each node has:

```c
struct node {
    int data;
    node *prev;
    node *next;
};
```

That means each node knows both:

- who comes after it
- who comes before it

#### A.2.2 `list_init()`

For a circular list, a single node points to itself in both directions.

```c
head->next = head;
head->prev = head;
```

#### A.2.3 `list_add()`

Last week, in our original singly linked list, adding to the end required walking through the whole list:

```c
while (curr->next != NULL) {
    curr = curr->next;
}
```

That was `O(n)`. In the circular doubly linked list, the tail is always:

```c
head->prev
```

So we can append in `O(1)` time. Suppose the list is:

```text
10 <-> 20 <-> 30
^              |
|______________|
```

Then:

- `head` points to `10`
- `head->prev` points to `30`, the tail

When adding `40`, we insert it between the tail and the head:

```c
node *tail = (*head)->prev;

new_node->next = *head;
new_node->prev = tail;

tail->next = new_node;
(*head)->prev = new_node;
```

After that:

```text
10 <-> 20 <-> 30 <-> 40
^                     |
|_____________________|
```

So the tail becomes the new node.

#### A.2.4 `list_delete()`

This is the biggest improvement over the singly linked list. In the old singly linked list, to delete a node we had to search for the node before it. Here, because every node has `prev`, deletion is direct. There are two cases.

##### Case A: Deleting The Only Node

If the list has just one node, then:

```c
n->next == n && n->prev == n
```

That means after deleting it, the list becomes empty:

```c
*head = NULL;
free(n);
```

##### Case B: Deleting a Node From a Larger List

We just "skip over" the node:

```c
n->prev->next = n->next;
n->next->prev = n->prev;
```

That reconnects its neighbours.

If the deleted node is the head, we also move the head forward:

```c
if (*head == n) {
    *head = n->next;
}
```

Then we free the node.

This deletion is `O(1)` as long as we are given the pointer to the exact node.

That is a great teaching point:

- singly linked list delete by pointer: usually `O(n)`
- doubly linked circular list delete by pointer: `O(1)`

#### A.2.5 `list_next()`

This function stays simple:

```c
return n->next;
```

But the meaning is slightly different now. In a normal singly linked list, the last node's next pointer is `NULL`. In a circular list, the last node's next pointer is the head. So for a non-empty circular list, `list_next` never returns `NULL`.

```c
node *tail = head->prev;
node *again = list_next(tail);
```

Now `again == head`.

#### A.2.6 `list_print()`

This is where students often make mistakes. In the original singly linked list, printing used:

```c
while (curr != NULL)
```

That worked because the list ended with `NULL`. However, in a circular list, that would never stop.So we use a `do ... while` loop:

```c
const node *curr = head;
do {
    ...
    curr = curr->next;
} while (curr != head);
```

Because if the list is non-empty, we must print the head node first, and only then check whether we have come back to it.

#### A.2.7 `list_free()`

Freeing the whole list also changes because there is no `NULL` at the end.

So the pattern is:

- remember the starting node
- move through the circle until we return to the start
- free each node
- finally free the starting node
- set `*head = NULL`

To avoids an infinite loop:

```c
node *start = *head;
node *curr = start->next;

while (curr != start) {
    ...
}
```

---

### A.3 Exercise: Detect Cycle in Linked List

> <https://leetcode.com/problems/linked-list-cycle/description/>

If here is a cycle in the linked list, the fast pointer would catching the slow one from behind. We need to make sure that `fast != NULL` and `fast->next != NULL`, cause we put `fast = fast->next->next` in the last iteration.

```c
bool hasCycle(struct ListNode *head) {
    if (head == NULL) return false;
    struct ListNode *fast = head->next;
    struct ListNode *slow = head;
    while (fast != slow) {
        if (fast == NULL || fast->next == NULL) {
            return false;
        }
        fast = fast->next->next;
        slow = slow->next;
    }
    return true;
}
```

---

### A.4 Exercise: Dynamic Array

> [!IMPORTANT]
> Refer to [`dyn_array.c`](./Codes/dyn_array.c) for the code used in this section.

This exercise is the array version of the linked-list task. The key idea is that a **dynamic array** keeps two numbers:

- `size`: how many elements are actually being used
- `capacity`: how many elements can fit in the current allocation

and one pointer:

- `contents`: the heap-allocated block holding the integers

#### A.4.1 Define the Structure

```c
typedef struct dyn_array dyn_array;

struct dyn_array {
    int size;
    int capacity;
    int *contents;
};
```

For example, if

```text
size = 3
capacity = 4
contents = [10, 20, 30, ?]
```

then only the first 3 values are part of the array. The last slot is just spare space.

#### A.4.2 Pick a Default Capacity

When we first create the array, we need some starting space. A small default like 4 is fine.

```c
#define INITIAL_CAPACITY 4
```

This means a new dynamic array starts empty, but already has room for 4 integers.

#### A.4.3 Resizing Helper

When the array becomes full, we need a bigger heap allocation. The standard pattern is to **double** the capacity.

To avoid rewriting that logic inside `dyn_array_add`, it is cleaner to make a helper function:

```c
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
```

Important idea: `realloc` may move the array to a new memory location, so we must use the pointer it returns.

#### A.4.4 `dyn_array_init()`

This creates the structure itself and also allocates the first contents array.

```c
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
```

After this:

- `size = 0`
- `capacity = 4`
- `contents` points to space for 4 integers

#### A.4.5 `dyn_array_add()`

Adding is done at the end. If the array is full, resize first. Then place the value at index `size`, and increment `size`.

```c
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
```

Example:

If the array is:

```text
size = 3
capacity = 4
contents = [10, 20, 30, ?]
```

then `dyn_array_add(dyn, 40)` gives:

```text
size = 4
capacity = 4
contents = [10, 20, 30, 40]
```

If we now add one more element, the array is full, so it grows:

```text
before add: size = 4, capacity = 4
after resize: capacity = 8
after storing 50: size = 5
```

#### A.4.6 `dyn_array_get()`

Getting an element is easy because arrays allow direct indexing.

```c
#include <assert.h>

int dyn_array_get(dyn_array *dyn, int index) {
    assert(dyn != NULL);
    assert(index >= 0 && index < dyn->size);

    return dyn->contents[index];
}
```

So:

```c
int x = dyn_array_get(dyn, 2);
```

returns the value at index 2.

#### A.4.7 `dyn_array_set()`

The prompt mentions that the structure should support `set`, even though no prototype was listed. So this is a useful extra function:

```c
void dyn_array_set(dyn_array *dyn, int index, int value) {
    assert(dyn != NULL);
    assert(index >= 0 && index < dyn->size);

    dyn->contents[index] = value;
}
```

Example:

```c
dyn_array_set(dyn, 1, 99);
```

changes the element at index 1 to `99`.

#### A.4.8 `dyn_array_delete()`

Deleting an element from an array is different from deleting a node in a linked list. We cannot just “unlink” it. We must shift all later elements one position left.

```c
void dyn_array_delete(dyn_array *dyn, int index) {
    if (dyn == NULL || index < 0 || index >= dyn->size) {
        return;
    }

    for (int i = index; i < dyn->size - 1; i++) {
        dyn->contents[i] = dyn->contents[i + 1];
    }

    dyn->size--;
}
```

Example:

Before:

```text
index:    0   1   2   3
contents: 10  20  30  40
size = 4
```

After `dyn_array_delete(dyn, 1)`:

```text
contents: 10  30  40  40
size = 3
```

Only the first `size` elements matter, so the array is now logically:

```text
[10, 30, 40]
```

The old last slot is ignored.

#### A.4.9 `dyn_array_free`

Because both the `contents` array and the `dyn_array` structure itself were allocated on the heap, we must free both.

```c
void dyn_array_free(dyn_array *dyn) {
    if (dyn == NULL) {
        return;
    }

    free(dyn->contents);
    free(dyn);
}
```

This is the delete the entire allocation operation from the prompt.

#### A.4.10 Optional Extension: Shrinking

The above version above grows when full, but does not shrink when many elements are deleted. For example, add this near the end of `dyn_array_delete`:

```c
if (dyn->capacity > INITIAL_CAPACITY && dyn->size <= dyn->capacity / 4) {
    int new_capacity = dyn->capacity / 2;
    if (new_capacity < INITIAL_CAPACITY) {
        new_capacity = INITIAL_CAPACITY;
    }
    dyn_array_resize(dyn, new_capacity);
}
```

---

### A.5 Void Pointers

A `void *` is a **generic object pointer** in C. The C standard says the `void` type has no values and is an incomplete object type, and it also says a pointer to any object type may be converted to `void *` and back again, with the round-trip pointer comparing equal to the original pointer. In practice, that means `void *` is the language's "I have an address, but I'm not committing to the pointed-to object type here" pointer. Please note that a `void *` does **not** magically remember the real type at runtime. It can hold the address of any object, but the function using it must get the real type from somewhere else, such as an extra parameter, an enum tag, a size argument, or a callback. The Linux `void(3type)` page says exactly that: `void *` is useful for generic parameters and return values, but the function typically needs some other mechanism to know the real type.

#### A.5.1 `void *` in C

In standard C, conversions between `void *` and **object pointers** are special. We can convert an `int *`, `double *`, `struct node *`, and so on to `void *`, and back again. The standard also allows assignment between an object pointer and a `void *` in ordinary C code, which is why `malloc` can return `void *` and still be assigned directly to `int *` in C.

There is one subtle limit worth teaching clearly: the standard guarantee is about **object pointers**, not function pointers. POSIX goes further and requires that function pointers can also be converted to `void *` and back, but portable standard C only guarantees this for object pointers.

Another useful fact is that the standard says a `void *` has the same representation and alignment requirements as a character pointer. That is part of why `void *` works well as a generic "pointer-shaped" value in APIs.

We can store object addresses in a `void *`, pass them to functions, return them from functions, compare them with object pointers using `==` and `!=`, and convert them back to the original object-pointer type before using the pointed-to object. The standard also permits comparisons between an object pointer and a `void *` by converting the object pointer to the `void *` type for the comparison.

For example,

```c
int x = 42;
void *vp = &x;      // int * -> void *

int *ip = vp;       // void * -> int *
printf("%d\n", *ip);
```

This works because the stored address still points to the same `int`; `void *` only made the type generic temporarily.

However, we cannot directly dereference a `void *` in standard C, because dereferencing it would produce an object of type `void`, and `void` is not a complete object type we can actually read or write as a normal value. The Linux manual states this directly: a `void *` cannot be dereferenced.

So this is wrong:

```c
void *vp = &x;
printf("%d\n", *vp);   // WRONG
```

We must cast it back first:

```c
printf("%d\n", *(int *)vp);
```

Pointer arithmetic on `void *` is also not part of standard C. The standard's additive-operator rule requires a pointer to a **complete object type** for pointer-plus-integer arithmetic, and `void` is incomplete. The Linux `void(3type)` page adds that GNU C allows `void *` arithmetic as an extension, but that is not portable C.

#### A.5.2 Why `malloc()` Returns `void *`

`malloc()` allocates raw memory and returns a pointer to that allocated memory. Its return type is `void *` because `malloc()` does not know whether we plan to store `int`, `double`, `struct node`, or anything else there. The pointer it returns is documented as suitable for any type that fits in the requested size, and in C we can assign it directly to an object pointer without an explicit cast.

```c
int *arr = malloc(10 * sizeof *arr);
int *arr = (int *)malloc(10 * sizeof *arr);    // correct but unnecessary
```

#### A.5.3 `const void *`

You will often see `const void *`, especially in `qsort`, `bsearch`, and byte-copy/search functions.

`const void *` means "generic pointer to data that this function promises not to modify." It is the read-only version of `void *`. In `qsort`, the comparator receives pointers to elements through `const void *` parameters.

A tiny example:

```c
void print_number(const void *p) {
    printf("%d\n", *(const int *)p);
}
```

This says the pointed-to object is being read, not changed.

---

### A.6 Generic Lists

To make a linked list hold different kinds of values in C, the simplest approach is a tagged union. Each node contains an enum field that records the type of the current value, and a union that stores the actual value. The enum is necessary because a union only stores one active member at a time. For example, a node can store either an `int`, a `double`, or a `char *`. When printing or freeing the list, the program checks the enum first and then accesses the matching union member.

> [!IMPORTANT]
> Refer to [`generic_list.c`](./Codes/generic_list.c) for the code used in this section.
