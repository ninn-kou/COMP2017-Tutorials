## COMP2017 2026 S1 Week 4 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 4 Tutorial B](#comp2017-2026-s1-week-4-tutorial-b)
  - [B.1 Structs in C](#b1-structs-in-c)
    - [B.1.1 Declaring a Struct Variable](#b11-declaring-a-struct-variable)
    - [B.1.2 Initialising a Struct](#b12-initialising-a-struct)
    - [B.1.3 Accessing Fields](#b13-accessing-fields)
    - [B.1.4 Pointers to structs](#b14-pointers-to-structs)
    - [B.1.5 Worked example](#b15-worked-example)
    - [B.1.6 Why Structs Must Have a Fixed Size?](#b16-why-structs-must-have-a-fixed-size)
    - [B.1.7 Structs and Other Structs](#b17-structs-and-other-structs)
    - [B.1.8 *Struct Padding*](#b18-struct-padding)
  - [B.2 Enumeration Type in C](#b2-enumeration-type-in-c)
    - [B.2.1 Size of Enumeration](#b21-size-of-enumeration)
  - [B.3 Exercise: Struct Properties](#b3-exercise-struct-properties)
    - [B.3.1 Which of the Following Code Snippets Compile?](#b31-which-of-the-following-code-snippets-compile)
    - [B.3.2 What Assumptions Can You Make About `sizeof(struct pokemon)`?](#b32-what-assumptions-can-you-make-about-sizeofstruct-pokemon)
    - [B.3.3 What Does the Following Code Do?](#b33-what-does-the-following-code-do)
    - [B.3.4 And What Does the Following Code Do?](#b34-and-what-does-the-following-code-do)
  - [B.4 `typedef struct`](#b4-typedef-struct)
  - [B.5 Unions](#b5-unions)
    - [B.5.1 Size of a Union](#b51-size-of-a-union)
    - [B.5.3 What Happens if You Read a Different Member?](#b53-what-happens-if-you-read-a-different-member)
    - [B.5.4 Initialising a Union](#b54-initialising-a-union)
  - [B.6 Exercise: Run-length Tuples](#b6-exercise-run-length-tuples)
  - [B.7 Exercise: Cricket](#b7-exercise-cricket)
  - [B.8 Exercise: A Shape Interface](#b8-exercise-a-shape-interface)

---

### B.1 Structs in C

A `struct` lets us group several variables together into one larger value. Each part of the struct is called a field or member.

This is useful when several pieces of data belong together. For example, a point on a 2D grid has an `x` coordinate and a `y` coordinate, so it makes sense to store them in one struct instead of using two unrelated variables.

```C
struct coordinate {
    int x;
    int y;
};
```

This defines a new struct type called `struct coordinate`.

It does not create a variable yet. It only says, "a `coordinate` consists of two `int`s called `x` and `y`."

#### B.1.1 Declaring a Struct Variable

Once the type exists, we can create variables of that type.

```C
struct coordinate point;
```

Now `point` is a variable with two fields inside it: `point.x` and `point.y`.

**We can assign values to those fields using the dot operator `.`**

```C
point.x = 0;
point.y = 0;

printf("%d %d\n", point.x, point.y);
```

Expected output:

```C
0 0
```

The dot operator is used when we have the **actual** struct value.

#### B.1.2 Initialising a Struct

Like arrays, structs can be initialised using curly braces.

```C
struct coordinate point = { 0, 0 };
```

This means `x = 0` and `y = 0`. A clearer version is to name the fields explicitly.

```C
struct coordinate point = { .x = 0, .y = 0 };
```

This is called a designated initializer. It is often easier to read, and it avoids mistakes when a struct has many fields.

> [!CAUTION]
> Designated initializers are a C99 feature and later.

#### B.1.3 Accessing Fields

Just like what how we initialized the struct, each field has a name, and we access it with the dot operator.

```C
point.x
point.y
```

So if `point` stores `{3, 7}`, then:

- `point.x` is `3`
- `point.y` is `7`

#### B.1.4 Pointers to structs

We can also have pointers to structs, just like pointers to `int`, `char`, or any other type.

```C
struct coordinate* point_ptr = &point;
```

Now `point_ptr` stores the address of `point`.

To access a field through a pointer, we could write:

```C
(*point_ptr).x
```

This means:

1. dereference `point_ptr` to get the struct
2. access its `x` field

Because this is common, C provides a shorthand:

```C
point_ptr->x
```

These two are equivalent:

```C
(*point_ptr).x;
point_ptr->x;
```

**The arrow operator `->` is used when we have a pointer to a struct.**

A common beginner mistake is forgetting the parentheses in `(*point_ptr).x`. Writing `*point_ptr.x` is wrong, because `.` is evaluated before `*`.

#### B.1.5 Worked example

> [!IMPORTANT]
> Refer to [`struct.c`](./Codes/struct.c) for the code used in this section.

Output:

```C
point = (2, 4)
point = (10, 20)
```

#### B.1.6 Why Structs Must Have a Fixed Size?

A struct must have a size the compiler can determine at compile time. This is important because the compiler needs to know how much memory to allocate for each variable of that type.

For example:

```C
struct coordinate {
    int x;
    int y;
};
```

If an `int` is 4 bytes, then this struct is usually 8 bytes, though sometimes padding may affect the total size.

Because the size must be known in advance, a struct cannot directly contain something whose size is unknown at compile time.

For example, this is not allowed:

```C
int n = 5;
struct bad_example {
    int arr[n];   // not allowed
};
```

The size of `arr` depends on `n`, so the compiler cannot determine the struct size when compiling.

Instead, the struct can store a pointer:

```C
struct better_example {
    int* arr;
};
```

The pointer itself has a fixed size, so this is allowed. The actual array can live somewhere else in memory.

#### B.1.7 Structs and Other Structs

A struct can contain another struct, as long as that struct type is already known.

```C
struct coordinate {
    int x;
    int y;
};

struct rectangle {
    struct coordinate top_left;
    struct coordinate bottom_right;
};
```

This works because `struct coordinate` has already been defined.

However, a struct cannot directly contain itself, because that would make its size infinite.

This is invalid:

```C
struct node {
    int value;
    struct node next;   // invalid
};
```

Why invalid? Because `struct node` would need to contain another full `struct node`, which would contain another full `struct node`, and so on forever.

But a pointer to itself is allowed:

```C
struct node {
    int value;
    struct node* next;
};
```

This works because a pointer has a fixed size. This pattern is the basis of linked lists.

#### B.1.8 *Struct Padding*

> This B.1.8 part is copied from <https://www.w3schools.com/c/c_structs_padding.php>.

When you create a struct in C, the compiler may add some extra bytes of padding between members.

This is done to make the program run faster on your computer, because most CPUs read data more efficiently when it's properly aligned in memory.

Padding improves access efficiency, but it can make structs larger than the sum of their fields.

Let's look at a simple struct:

> [!IMPORTANT]
> Refer to [`struct_size.c`](./Codes/struct_size.c) for the code used in this section.

You might expect the size to be `1 + 4 + 1 = 6` bytes - but it will usually print `12` bytes! **Why?**

The compiler adds padding bytes so that the int member `b` starts at a memory address that's a multiple of `4`. This helps the CPU read it faster.

Here's how memory is actually arranged:

```text
Member     Bytes    Notes
a          1        Stored first
padding    3        Added so b starts at a multiple of 4
b          4        Aligned to 4-byte boundary
c          1        Stored next
padding    3        Added to make total size a multiple of 4
```

`Total = 1 + 3 + 4 + 1 + 3 = 12 bytes.`

---

### B.2 Enumeration Type in C

An `enum` in C is a way to give names to **a set of integer constants**. It makes code easier to read than using raw numbers.

```C
enum TYPE { FIRE, WATER, FLYING, ROCK, ELECTRIC };
```

Here, the names are assigned integer values automatically:

```C
FIRE = 0
WATER = 1
FLYING = 2
ROCK = 3
ELECTRIC = 4
```

You can declare variables of that enum type like this:

```C
enum TYPE pikachu_type = ELECTRIC;
```

This is much clearer than writing which using the "magic number":

```C
int pikachu_type = 4;
```

You can also give specific values yourself:

```C
enum status { FAIL = -1, OK = 0, WARN = 1 };
```

#### B.2.1 Size of Enumeration

`sizeof(enum TYPE)` is the number of bytes used to store an object of that enum type, not the number of enum constants. For an ordinary `enum`, C leaves the actual integer type implementation-defined: it must be compatible with some integer type wide enough to represent all the enumerator values.

```C
printf("%zu\n", sizeof(enum TYPE));
```

**Do not assume a fixed answer in portable C.** Some compilers make it the size of `int`, but compilers and ABIs can choose differently, and GCC's `-fshort-enums` explicitly shrinks an enum to the smallest integer type that fits. In C23, you can also give an enum a fixed underlying type, and then the enum is compatible with that type.

---

### B.3 Exercise: Struct Properties

The following questions are based on the following code snippet:

```C
enum TYPE { FIRE, WATER, FLYING, ROCK, ELECTRIC };

struct pokemon {
    const char* name;
    enum TYPE type;
};
```

#### B.3.1 Which of the Following Code Snippets Compile?

```text
(a) pokemon pikachu = { "Pikachu", ELECTRIC };
(b) struct pokemon pikachu = { ELECTRIC, "Pikachu" };
(c) struct pokemon pikachu = { "Pikachu", ELECTRIC };
(d) struct pokemon pikachu = { .type = ELECTRIC, .name = "Pikachu" };
(e) struct pokemon blank = { 0 };
```

(c), (d) and (e). For (e), we have:
- the first field `name` gets `0`, which is treated as a null pointer constant
- the remaining fields are zero-initialized automatically

So after this:

```C
blank.name == NULL
blank.type == 0   // which means FIRE
```

Because `FIRE` is the first enum constant, its value is `0`.

#### B.3.2 What Assumptions Can You Make About `sizeof(struct pokemon)`?

> [!WARNING]
> **You cannot assume one exact number across all systems.** The only thing we know is its size must be large enough to store both of those, plus any padding added by the compiler for alignment. **If anyone told you it must be 12, they are wrong.** Refer to B.1.8 for more information.

#### B.3.3 What Does the Following Code Do?

```C
struct pokemon pikachu = { "Pikachu", ELECTRIC };
struct pokemon *ptr = &pikachu;
ptr->name = "Raichu";
ptr->type = ELECTRIC;
```

`ptr` points to `pikachu`, so using `ptr->...` changes the original struct.

After this code runs, `pikachu` becomes effectively:

```C
{ "Raichu", ELECTRIC }
```

A very important detail is that:

```C
const char* name;
```

means "pointer to constant characters."

That means:
- you **can** change the pointer so it points somewhere else
- you **cannot** use it to modify the characters in the string literal

So this is allowed:

```C
ptr->name = "Raichu";
```

but something like this would not be allowed:

```C
ptr->name[0] = 'R';   // invalid: modifying const data
```

#### B.3.4 And What Does the Following Code Do?

```C
void evolve(struct pokemon mon) {
    mon.name = "Raichu";
    mon.type = ELECTRIC;
}

int main() {
    struct pokemon pikachu = { "Pikachu", ELECTRIC };
    evolve(pikachu);
}
```

The function parameter `struct pokemon mon` is passed **by value**, so `mon` is a copy of `pikachu`. That means `evolve` modifies only its local copy, not the original variable in `main`.

So after `evolve(pikachu);` the original `pikachu` in `main` is still: `{"Pikachu", ELECTRIC}`.

The changes inside the function disappear when the function returns.

> [!IMPORTANT]
> Passing a struct to a function copies the whole struct. Changes to the parameter affect only the copy unless the function receives a pointer to the struct.

There is one more subtle idea worth noting.

Because the struct contains a pointer, copying the struct copies the pointer value, not the string itself. So initially `mon.name` and `pikachu.name` point to the same string literal. But in `evolve`, this line:

```C
mon.name = "Raichu";
```

only changes the copied pointer inside `mon`. It does not change `pikachu.name`.

**How to change `evolve` so it modifies the original object?**

The standard fix is to pass a pointer:

```C
void evolve(struct pokemon *mon) {
    mon->name = "Raichu";
    mon->type = ELECTRIC;
}

int main(void) {
    struct pokemon pikachu = { "Pikachu", ELECTRIC };
    evolve(&pikachu);
}
```

Now `mon` points to the original struct, so the function updates the real object.

After the call, `pikachu` is:

```C
{ "Raichu", ELECTRIC }
```

Another correct approach is to return the modified struct and assign it back:

```C
struct pokemon evolve(struct pokemon mon) {
    mon.name = "Raichu";
    mon.type = ELECTRIC;
    return mon;
}

int main(void) {
    struct pokemon pikachu = { "Pikachu", ELECTRIC };
    pikachu = evolve(pikachu);
}
```

This also works, but it is not modifying the original object through the function call itself. Instead, it returns a new value and the caller replaces the old one.

---

### B.4 `typedef struct`

---

### B.5 Unions

A `union` is similar to a `struct`, but with one major difference:
- In a `struct`, each field has its own separate storage in memory.
- In a `union`, all fields share the same region of memory.
That means a union can hold different kinds of data, but only one member is meaningfully stored at a time.

```C
union number {
    int i;
    float f;
    double d;
};
```

This defines a union named `union number` with three possible views of the same memory:
- as an `int`
- as a `float`
- as a `double`

A `union` stores them in the same place. So for a `union`:
- writing to `i` overwrites the memory used by `f` and `d`
- writing to `f` overwrites the memory used by `i` and `d`
- writing to `d` overwrites the memory used by `i` and `f`

#### B.5.1 Size of a Union

The size of a union is large enough to hold its largest member, possibly with padding for alignment.

So for:

```C
union number {
    int i;
    float f;
    double d;
};
```

we can assume:

```C
sizeof(union number) >= sizeof(double)
```

On many systems, `sizeof(union number)` will be the same as `sizeof(double)`, but the exact value is implementation-dependent.

Let's go through this carefully.

**Step 1**

```C
n.i = 10;
```

The shared memory now contains the bit pattern for the integer `10`. At this point, the meaningful member is `n.i`.

**Step 2**

```C
n.f = 10.05;
```

Now the same memory is overwritten with the bit pattern for the float `10.05`. This destroys the previous integer representation. After this line:
- `n.f` is meaningful
- `n.i` is no longer the integer `10`

**Step 3**

```C
n.d = 12.02;
```

Now the same memory is overwritten again, this time with the bit pattern for the double `12.02`. After this line:
- `n.d` is the meaningful value
- `n.i` and `n.f` are just different interpretations of the same bytes

**So the final assignment decides what data is actually stored.**

#### B.5.3 What Happens if You Read a Different Member?

Suppose we do:

```C
n.d = 12.02;
printf("%d\n", n.i);
```

This does **not** convert `12.02` into an integer.

Instead, it interprets some of the bytes of the stored `double` as if they were an `int`.

**That means the output is not a numeric conversion. It is a reinterpretation of raw memory, and the result is usually meaningless for normal program logic.**

For examplem, `(int)n.d` will force convert `12.02` to `12`, but `n.i` simply treats the bytes currently stored in memory as an `int`, which gives some implementation-dependent result.

#### B.5.4 Initialising a Union

A union can also be initialised.

```C
union number n1 = { 10 };
```

This initialises the first member, so it means:

```C
n1.i = 10;
```

You can also use a designated initializer:

```C
union number n2 = { .d = 12.02 };
```

This makes it clear which member is intended to be active.

---

### B.6 Exercise: Run-length Tuples

> [!IMPORTANT]
> Refer to [`tuple_encode.c`](./Codes/tuple_encode.c) for the code used in this section.

---

### B.7 Exercise: Cricket

Because `first_name` and `last_name` are `char *`, the struct only stores pointers. That means we must allocate memory for each name before storing it. A clean solution is to read each name into a temporary array, then `malloc` exactly enough space and copy the name into the struct.

> [!IMPORTANT]
> Refer to [`batsman.c`](./Codes/batsman.c) for the code used in this section.

`scanf("%100s %100s %d", ...)` reads a first name, a last name, and a score. The `%100s` prevents buffer overflow because the temporary arrays are size 101. `copy_name` allocates enough memory for the string and copies it. This is necessary because the struct fields are pointers, not built-in character arrays. When printing, `player->first_name[0]` gives the first initial, so `"Sam Konstas"` becomes `S. Konstas`. If the score is `0`, the program prints `Duck` instead of the number.

---

### B.8 Exercise: A Shape Interface

> [!IMPORTANT]
> Refer to [`nested.c`](./Codes/nested.c) for the code used in this section.
