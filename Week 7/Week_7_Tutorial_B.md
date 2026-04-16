## COMP2017 2026 S1 Week 7 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 7 Tutorial B](#comp2017-2026-s1-week-7-tutorial-b)
  - [B.1 Compilation: The C Pre Processor](#b1-compilation-the-c-pre-processor)
  - [B.2 Compilation: The Compiler](#b2-compilation-the-compiler)
  - [B.3 Compilation: The Assembler](#b3-compilation-the-assembler)
  - [B.4 Compilation: The Linker](#b4-compilation-the-linker)
  - [B.5 External Linkage \& Internal Linkage](#b5-external-linkage--internal-linkage)
    - [B.5.1 External Linkage](#b51-external-linkage)
    - [B.5.2 Internal Linkage](#b52-internal-linkage)
  - [B.6 Keywords: `extern`, `static`](#b6-keywords-extern-static)
    - [B.6.1 `extern`](#b61-extern)
      - [Functions and `extern`](#functions-and-extern)
    - [B.6.2 `static`](#b62-static)
      - [`static` at File Scope](#static-at-file-scope)
      - [`static` Inside a Function](#static-inside-a-function)
  - [B.7 Keywords: `const` and `restrict`](#b7-keywords-const-and-restrict)
    - [B.7.1 `const`](#b71-const)
    - [B.7.2 `restrict`](#b72-restrict)
  - [B.8 Exercise: Trees](#b8-exercise-trees)
    - [B.8.1 Tree Structure](#b81-tree-structure)
    - [B.8.2 `create_node()`](#b82-create_node)
    - [B.8.3 `add_child()`](#b83-add_child)
    - [B.8.4 Depth-First Search](#b84-depth-first-search)
    - [B.8.5 `find_node()`](#b85-find_node)
    - [B.8.6 `destroy_tree()`](#b86-destroy_tree)
  - [B.9 Exercise: Basic File System](#b9-exercise-basic-file-system)
  - [B.10 Exercise: Market](#b10-exercise-market)
    - [B.10.1 Choose the Outer Structure](#b101-choose-the-outer-structure)
    - [B.10.2 Choose the Inner Structure](#b102-choose-the-inner-structure)
    - [B.10.3 Input Workflow](#b103-input-workflow)
    - [B.10.4 Teardown Workflow](#b104-teardown-workflow)
  - [B.11 Exercise: Tiny Delegator](#b11-exercise-tiny-delegator)
    - [B.11.1 Represent One Instruction](#b111-represent-one-instruction)
    - [B.11.2 Parse Each Line](#b112-parse-each-line)
    - [B.11.3 Execute the List](#b113-execute-the-list)
  - [B.12 Exercise: Generic HashMap](#b12-exercise-generic-hashmap)
    - [B.12.1 Separate Chaining Design](#b121-separate-chaining-design)
    - [B.12.2 Map Design](#b122-map-design)
    - [B.12.3 Bucket Lookup](#b123-bucket-lookup)
    - [B.12.4 Resizing](#b124-resizing)

---

### B.1 Compilation: The C Pre Processor

The preprocessor runs before the real compiler. It handles things like `#include`, `#define`, and conditional compilation. A useful way to think about it is: the preprocessor mainly rewrites the source text before C compilation really begins.

It resolves:

- `#include`
- `#define`
- `#ifdef`, `#ifndef`, `#endif`
- generally, anything starting with `#`

These are not ordinary C statements.

```c
#define SIZE 4
int arr[SIZE];
```

After preprocessing, this is effectively:

```c
int arr[4];
```

To inspect the preprocessed output:

```bash
cpp tasks.c
```

and

```bash
gcc -E tasks.c
```

This is especially useful when macros behave strangely.

---

### B.2 Compilation: The Compiler

After preprocessing, the compiler translates C into assembly.

Assembly is still human-readable text, but it is architecture-specific. That means it is no longer portable C source.

A short example command:

```bash
gcc -S -g -std=c11 -Wall -Werror demo.c
```

This produces something like:

```text
demo.s
```

That `.s` file contains assembly instructions for the target CPU.

For following codes,

```c
#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 1;
    int y = 2;
    return add(x, y);
}

```

it become assembly instructions that load values into registers, perform an add instruction, and return.

---

### B.3 Compilation: The Assembler

The assembler takes assembly code and turns it into an object file.

To compile and assemble into an object file:

```bash
gcc -c -g -std=c11 -Wall -Werror list.c
```

This produces something like:

```text
list.o
```

An object file contains machine code, but usually also symbol information and relocation information. It is not yet a full executable program.

You can inspect object files with:

```bash
objdump -M intel -S list.o
```

We could understand these files as:
- `.c` file = source code
- `.s` file = assembly text
- `.o` file = assembled object code

Also, each `.c` file is a separate translation unit, so in a multi-file program you usually get one object file per `.c` file.

---

### B.4 Compilation: The Linker

The linker combines object files into a final executable.

If `main.c` calls a function defined in `helpers.c`, the linker is the stage that connects those together.

Example:

```bash
gcc -c main.c
gcc -c helpers.c
gcc main.o helpers.o -o app
```

If the linker cannot find a required symbol, you get an "undefined reference" error.

In summary, we could say
- the compiler translates each translation unit separately,
- the linker joins them together at the end.

---

### B.5 External Linkage & Internal Linkage

Linkage is about whether the **same name** in different translation units refers to the **same thing**. A translation unit is basically one `.c` file after preprocessing. So if a project has:

```text
main.c
helper.c
```

then there are two translation units. When both files are compiled, the linker decides whether names from one file can connect to names from another file.

- **external linkage**: the name can be shared across `.c` files
- **internal linkage**: the name is private to one `.c` file only

#### B.5.1 External Linkage

A name with external linkage can be used from another translation unit. At file scope, ordinary functions and ordinary global variables usually have external linkage by default.

```c
/* file1.c */
int global_count = 10;
```

```c
/* file2.c */
extern int global_count;

int main(void) {
    return global_count;
}
```

- `file1.c` contains the real definition of `global_count`
- `file2.c` uses `extern` to say "this variable exists somewhere else"

Because `global_count` has external linkage, the linker can connect them. So external linkage means "**this name is visible outside the current translation unit**".

#### B.5.2 Internal Linkage

A name with internal linkage is only visible inside the current translation unit. At file scope, this is usually created by `static`.

```c
/* helper.c */
static int secret_value = 42;
```

Now `secret_value` can be used inside `helper.c`, but another file cannot refer to it by name.

For example, this will fail:

```c
/* main.c */
extern int secret_value;   // wrong if secret_value was static in helper.c
```

because `secret_value` is private to `helper.c`. So internal linkage means "**this name is private to one `.c` file**".

---

### B.6 Keywords: `extern`, `static`

#### B.6.1 `extern`

`extern` usually means the name exists, but its definition is somewhere else.

```c
/* config.c */
int max_users = 100;
```

```c
/* main.c */
extern int max_users;
```

This is the most common pattern. To avoids **duplicate definitions**, Only **one** `.c` file should contain the actual definition:

```c
int max_users = 100;
```

Other files should just declare it with `extern`:

```c
extern int max_users;
```

Another common header pattern is

```c
/* config.h */
#ifndef CONFIG_H
#define CONFIG_H

extern int max_users;

#endif
```

```c
/* config.c */
#include "config.h"

int max_users = 100;
```

```c
/* main.c */
#include <stdio.h>
#include "config.h"

int main(void) {
    printf("%d\n", max_users);
}
```

- put the `extern` declaration in the header
- put the real definition in exactly one `.c` file

##### Functions and `extern`

Function declarations are external by default, so this:

```c
int add(int a, int b);
```

already behaves like an external declaration.

Writing:

```c
extern int add(int a, int b);
```

is valid, but usually unnecessary.

#### B.6.2 `static`

`static` is one of the trickiest keywords in C because it means different things in different places. There are **two main cases**.

##### `static` at File Scope

At file scope, `static` gives **internal linkage**. That means the name becomes private to the current translation unit.

```c
/* math_utils.c */
static int square(int x) {
    return x * x;
}
```

Now `square` can only be called inside `math_utils.c`. That is very useful for helper functions that should not be part of the public interface.

For a *private global* variable, it could also be written. as

```c
/* counter.c */
static int current_id = 0;
```

Again, `current_id` is private to `counter.c`. A good rule is that if a file-scope function or variable does not need to be used from another `.c` file, make it `static`.

##### `static` Inside a Function

Inside a function, `static` does **not** mean internal linkage. Instead, it means the local variable keeps its value between function calls. For example, we have

```c
#include <stdio.h>

int next_id(void) {
    static int id = 0;
    id++;
    return id;
}

int main(void) {
    printf("%d\n", next_id());
    printf("%d\n", next_id());
    printf("%d\n", next_id());
}
```

Output:

```text
1
2
3
```

If `id` were an ordinary local variable:

```c
int next_id(void) {
    int id = 0;
    id++;
    return id;
}
```

then the output would be:

```text
1
1
1
```

because the variable would be recreated each time.

---

### B.7 Keywords: `const` and `restrict`

#### B.7.1 `const`

`const` means "do not modify through this name".

```c
const int MAX_SIZE = 100;
```

This should not be reassigned later.

With pointers, const can apply to the pointed-to value or to the pointer itself.

```c
const int *ptr = &x;
```

means:
- `*ptr` should not be modified through `ptr`
- `ptr` itself may point somewhere else later

```c
int * const ptr = &x;
```

means:

- `ptr` must always point to the same address
- the value at that address can still be modified

```c
const int * const ptr = &x;
```

means:
- the pointer cannot move
- the value cannot be changed through it

#### B.7.2 `restrict`

`restrict` is a promise about aliasing.

If a pointer is declared restrict, it means that for that scope, the object it points to will be accessed only through that pointer or values derived from it.

A short example:

```c
void add_arrays(int n,
                int * restrict out,
                const int * restrict a,
                const int * restrict b) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] + b[i];
    }
}
```

This promises that out, `a`, and `b` do not overlap in a way that breaks the restrict rule.

That helps the compiler optimize.

> [!CAUTION]
> - `restrict` is not enforced automatically.
> - It is a promise made by the programmer.

---

### B.8 Exercise: Trees

The task is to build a tree structure, support adding children, perform DFS, find a node, and destroy the tree.

> [!IMPORTANT]
> Refer to the folder [`tree/*`](./Codes/tree/) for codes.

```text
tree/
├── tree.h
├── tree.c
└── main.c
```

- `tree.h` contains the struct and function declarations
- `tree.c` contains the tree logic
- `main.c` just builds a sample tree and tests the functions

To compile:

```bash
gcc -std=c11 -Wall -Wextra -Werror tree.c main.c -o tree_demo
```

#### B.8.1 Tree Structure

To design a tree, we could say:
- each node stores one value
- each node stores a dynamic array of child pointers
- `child_count` tells how many children currently exist
- `child_capacity` tells how much space is allocated

Because the number of children is not known in advance, `realloc()` is useful for growing the child array.

```c
struct tree_node {
    int value;
    struct tree_node **children;
    size_t child_count;
    size_t child_capacity;
};
```

#### B.8.2 `create_node()`

This function needs to:

1. allocate memory for the node
2. store the value
3. initialize the children array to `NULL`
4. set the count and capacity to `0`

#### B.8.3 `add_child()`

When adding a child:

1. if the current array is full, grow it
2. append the child pointer
3. increase `child_count`

A common growth strategy is doubling the capacity.

#### B.8.4 Depth-First Search

The required output `A B E C F D` matches preorder DFS:

1. visit current node
2. recursively visit each child from left to right

#### B.8.5 `find_node()`

This is also naturally recursive:

- if current node matches, return it
- otherwise search each child
- return the first match found
- return `NULL` if nothing matches

#### B.8.6 `destroy_tree()`

To free the tree safely:

1. destroy all children first
2. free the child array
3. free the node itself

That is postorder destruction.

---

### B.9 Exercise: Basic File System

> [!IMPORTANT]
> Refer to the folder [`basic_fs/*`](./Codes/basic_fs/) for codes.

```text
basic_fs/
├── fs.h
├── fs.c
└── main.c
```

- `fs.h` contains the file-system node definition and public operations
- `fs.c` contains the tree-based file-system logic
- `main.c` implements the shell-style command loop

To compile:

```bash
gcc -std=c11 -Wall -Wextra -Werror fs.c main.c -o basic_fs
```

This task is open-ended, but the cleanest solution is to model the file system as a tree.

Each node is either a file or a directory. Directories can have children, while files are leaves. A good design is one struct for both.

For my solutions, I put

```c
typedef struct fs_node {
    char *name;
    int is_directory;
    struct fs_node *parent;
    struct fs_node **children;
    size_t child_count;
    size_t child_capacity;
} fs_node;
```

This gives me:

- `name`
- whether it is a directory
- a parent pointer for `cd ..`
- a dynamic array of children for directories

Each shell command is really just a tree operation.

- `touch <fname>` means create a file node and add it to the current directory.
- `mkdir <dname>` means create a directory node and add it to the current directory.
- `ls` means print the children of the current directory.
- `cd ..` means move to `parent`, unless already at root.
- `cd <name>` means find a child with that name and move into it if it is a directory.
- `rm <name>` means remove the child and destroy its whole subtree.
- `find <name>` means search the current subtree recursively.
- `tree` means print the subtree recursively with indentation.

Following ideas could be helpful when you implement your own codes.

Without `parent`, `cd ..` becomes awkward. It also makes it easy to print the current path like:

```text
>~/dir1/dir2
```

Meanwhile, if the removed node is a directory, all files and subdirectories under it should disappear too.

So `rm` really means:

- unlink child from parent
- destroy the subtree rooted at that child

---

### B.10 Exercise: Market

> [!IMPORTANT]
> Refer to the folder [`market/*`](./Codes/market/) for codes.

```text
market/
├── purchase_queue.h
├── purchase_queue.c
├── transactions.h
├── transactions.c
└── main.c
```

- `purchase_queue.*` manages the purchase priority queue
- `transactions.*` manages the customer linked list and receipt writing
- `main.c` handles input and drives the workflow

To compile:

```bash
gcc -std=c11 -Wall -Wextra -Werror purchase_queue.c transactions.c main.c -o market
```

#### B.10.1 Choose the Outer Structure

The outer structure stores each customer checkout.

A linked list is a good fit because checkouts happen one after another during the day.

A transaction node stores:

- customer name
- phone number
- checkout time
- pointer to the item structure
- next transaction pointer

#### B.10.2 Choose the Inner Structure

The exercise says items should be sorted by price.

A priority queue is a good fit for that. I can use a min-heap so the cheapest item comes out first.

Each purchase stores:

- item name
- quantity
- cost

#### B.10.3 Input Workflow

The workflow becomes:

1. read customer name
2. read phone number
3. read checkout time
4. keep reading item lines in the form
   `<item> <quantity> <cost>`
5. stop that customer when the cashier types `DONE`
6. repeat for the next customer until EOF

#### B.10.4 Teardown Workflow

At EOF:

1. traverse the linked list of transactions
2. create one file per transaction
3. pop the purchases out of the priority queue in sorted order
4. write the receipt
5. free everything

---

### B.11 Exercise: Tiny Delegator

> [!IMPORTANT]
> Refer to the folder [`delegator/*`](./Codes/delegator/) for codes.

```text
delegator/
├── ops.h
├── ops.c
├── program.h
├── program.c
└── main.c
```

- `ops.*` stores the arithmetic functions and operation parser
- `program.*` stores instruction parsing, execution, and cleanup
- `main.c` reads instructions from stdin and runs the program

To compile:

```bash
gcc -std=c11 -Wall -Wextra -Werror ops.c program.c main.c -o delegator
```

This task is about turning string instructions into executable operations. A natural design is:

- one linked list node per instruction
- each node stores a function pointer
- each node also stores the operands
- `%` means "use previous result"

#### B.11.1 Represent One Instruction


```c
typedef int (*binop_fn)(int, int);

struct instruction {
    binop_fn op;
    int lhs;
    int rhs;
    int lhs_is_prev;
    int rhs_is_prev;
    struct instruction *next;
};
```

This means each node stores:

- the operation
- two operands
- two flags saying whether either operand should come from the previous result
- the next instruction

#### B.11.2 Parse Each Line

For each line like:

```text
ADD 9 10
SUB % 10
MUL 3 %
DIV % 1
```

the parser needs to:

1. identify the operation
2. map it to the correct function pointer
3. parse operand 1
4. parse operand 2
5. record whether `%` was used

#### B.11.3 Execute the List

Execution is simple once the list is built:

- keep one `previous` variable
- if operand is `%`, use `previous`
- otherwise use the literal number
- call the function pointer
- store the result back into `previous`

---

### B.12 Exercise: Generic HashMap

> [!IMPORTANT]
> Refer to the folder [`hashmap/*`](./Codes/hashmap/) for codes.

```text
hashmap/
├── hashmap.h
├── hashmap.c
├── hash_utils.h
├── hash_utils.c
└── main.c
```

- `hashmap.*` stores the separate-chaining map implementation
- `hash_utils.*` stores reusable helper callbacks like `djb2`
- `main.c` is a small demo program

To compile:

```bash
gcc -std=c11 -Wall -Wextra -Werror hashmap.c hash_utils.c main.c -o hashmap_demo
```

This task wants a generic hashmap with:

- separate chaining
- resizing
- generic keys and values
- callback functions for hashing, comparison, and cleanup

My solution is an array of buckets, where each bucket is the head of a linked list.

#### B.12.1 Separate Chaining Design

A bucket array alone is not enough, because collisions happen. So each bucket stores a linked list of entries. Each entry stores:

- key
- value
- pointer to next entry in the chain

#### B.12.2 Map Design

The map itself stores:

- bucket array
- bucket count
- element count
- hash callback
- key delete callback
- value delete callback
- compare callback

#### B.12.3 Bucket Lookup

To insert or search a key:

1. compute the hash
2. take modulo by bucket count
3. walk that chain

That is the basic hashmap pattern.

#### B.12.4 Resizing

When the load factor gets too high, collisions increase. So the map should resize, usually by doubling the bucket count.

When resizing, every entry must be rehashed into the new bucket array, because the modulo result depends on bucket count.
