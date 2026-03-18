## COMP2017 2026 S1 Week 4 Tutorial A

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 4 Tutorial A](#comp2017-2026-s1-week-4-tutorial-a)
  - [A.1 String Length](#a1-string-length)
  - [A.2 String Compare](#a2-string-compare)
  - [A.3 Substring](#a3-substring)
  - [A.4 Run Length Encoding](#a4-run-length-encoding)
  - [A.5 2D Substring](#a5-2d-substring)
  - [A.6 C Maths Standard Library and `math.h`](#a6-c-maths-standard-library-and-mathh)
    - [A.6.1 Function naming: `name`, `namef`, and `namel`](#a61-function-naming-name-namef-and-namel)
    - [A.6.2 `exp2(x)`](#a62-exp2x)
    - [A.6.3 `fdim(x, y)`](#a63-fdimx-y)
    - [A.6.4 `fma(x, y, z)`](#a64-fmax-y-z)
    - [A.6.5 `fmax(x, y)` and `fmin(x, y)`](#a65-fmaxx-y-and-fminx-y)
    - [A.6.6 `hypot(x, y)`](#a66-hypotx-y)
    - [A.6.7 `lgamma(x)`](#a67-lgammax)
    - [A.6.8 `lrint()` and `llrint()`](#a68-lrint-and-llrint)
    - [A.6.9 `lround()` and `llround()`](#a69-lround-and-llround)
      - [A small rounding-mode demo](#a-small-rounding-mode-demo)
    - [A.6.9 `log1p(x)`](#a69-log1px)
  - [A.7 Volume of A Sphere](#a7-volume-of-a-sphere)
  - [A.8 Bits and Bitwise Operators](#a8-bits-and-bitwise-operators)
    - [A.8.1 Reading a binary value](#a81-reading-a-binary-value)
    - [A.8.2 Bitwise AND: `&`](#a82-bitwise-and-)
    - [A.8.3 Bitwise OR: `|` and XOR: `^`](#a83-bitwise-or--and-xor-)
    - [A.8.4 Bitwise NOT: `~`](#a84-bitwise-not-)
    - [A.8.5 Left shift: `<<` and right shift: `>>`](#a85-left-shift--and-right-shift-)
    - [A.8.6 Compound bitwise assignment](#a86-compound-bitwise-assignment)
    - [A.8.7 Common flag patterns](#a87-common-flag-patterns)
  - [A.9 Exercise: Retro Game Inventory Manager](#a9-exercise-retro-game-inventory-manager)

---

### A.1 String Length

The most important thing to understand about a C string is that it always ends at the first null terminator, which is written as `'\0'`. You can think of this as a stop sign for the computer. When you use the strlen function, it scans through the string and counts every character it sees before it hits that stop sign. It is worth noting that the function stops exactly at that marker, so it doesn't include the `'\0'` itself in the final count.

In the code, we start a counter `i = 0` and look at the string character by character. As long as the current character is not the null terminator, we move to the next one and increment our counter. The moment we reach the first `'\0'`, the loop stops and we return the current value of the counter. That returned value is exactly the length of the string.

> [!IMPORTANT]
> Refer to [`my_strlen.c`](./Codes/my_strlen.c) for the code used in this section.

It is easy to get `'\n'` and `'\0'` mixed up, but they serve completely different roles. A newline character is just a part of the text that moves the cursor to the next line, whereas the null terminator is a hidden signal used specifically to mark where the string ends. Additionally, the parameter is defined as const char *s to tell the compiler that this function should only read the string. This is a standard safety measure that prevents the function from accidentally changing any of the data it is measuring.

While we are using a standard int for this exercise to keep things simple, the real strlen function in the standard library actually returns a type called size_t. If you want to compare your results against the official library version, you should use the %zu format specifier in your print statements to display the standard result correctly. Finally, keep in mind that this function assumes the string actually has a null terminator. If that marker is missing, the function will keep searching through memory indefinitely, which causes undefined behavior and usually leads to a crash.

---

### A.2 String Compare

In our [Week 3 Tutorial A](../Week%203/Week_3_Tutorial_A.md) exercise "Word Searching", we used a simplified `string_compare` function that keeps moving forward while the current characters are equal and then returns the difference at the first mismatch. This exercise is the same idea, but with one extra stopping condition: stop after `len` characters. That is the same basic pattern as the standard `strncmp`, which compares at most `n` bytes and does not compare beyond the first NUL character.

The main idea is to compare one character at a time using an index `i`. The loop continues only while two things are true: we have not yet compared `len` characters, and the current characters are equal. If we reach `'\0'` while they still match, both strings ended at the same place, so the compared part is equal and we return `0`.

> [!IMPORTANT]
> Refer to [`my_strncmp.c`](./Codes/my_strncmp.c) for the code used in this section.

There are three ways the function can finish. If it reaches `len`, then the first `len` characters are equal, so it returns `0`. If it reaches `'\0'` in both strings before `len`, it also returns `0`. Otherwise it has found the first different character, so it returns the difference between them. A negative result means `w1` is smaller, a positive result means `w1` is larger, and zero means the compared prefix is equal. The cast to `unsigned char` makes this closer to the standard `strncmp` rule, which defines the sign of the result using byte values interpreted as `unsigned char`.

The extra tests show the edge cases clearly. `"Hello"` and `"Helium"` with `len = 3` return `0` because only `"Hel"` is checked. `"cat"` and `"catalog"` with `len = 3` also return `0`, but with `len = 4` the result becomes negative because the fourth character in `"cat"` is `'\0'` and the fourth character in `"catalog"` is `'a'`. If `len = 0`, the function compares zero characters and returns `0` immediately. If you check your answer against the library function `strncmp`, compare the sign of the result rather than relying on one exact non-zero number.

---

### A.3 Substring

Again, in C, a string is just a sequence of characters ending at `'\0'`, and the basic search strategy is to try every possible starting position in the larger string until one position matches the smaller string completely.

> [!IMPORTANT]
> Refer to [`my_strstr.c`](./Codes/my_strstr.c) for the code used in this section.

The outer loop chooses a starting index `i` in `line`. For each `i`, the inner loop compares `line[i + j]` with `substr[j]` one character at a time. If every character in `substr` matches, then eventually `substr[j]` becomes `'\0'`, which means the whole substring has been found, so we return `i`. If no starting position works, we return `-1`. This is the same “try every starting position” idea described in your notes.

A small walkthrough helps. For `substring("racecar", "car")`, the function first tries index `0`, then `1`, then `2`, then `3`. None of those starts match `"car"`. At index `4`, it compares `'c'`, `'a'`, and `'r'`, all match, so it returns `4`.

In our demonstration, we chose to return `0` for an empty substring. That matches the usual behavior of the standard library function `strstr`, which treats the empty string as being found at the start of the larger string. The standard `strstr` returns a pointer or `NULL`; this exercise uses the same search idea but returns an index or `-1` instead. This also explains the extra tests. `"banana"` with `"ana"` returns `1` because the first match starts at index `1`. `"aaaaa"` with `"aa"` returns `0` because we return the first occurrence, not the last one or all of them. That matches the usual first-occurrence substring search behavior.

---

### A.4 Run Length Encoding

For this problem, we could scan from left to right and count how many times the current character repeats in a row. Once the run ends, we write that count into the output buffer as a digit character. Then we jump forward by the whole run and repeat the same process for the next group. For the example `"1122333334423"`, the runs are `"11"`, `"22"`, `"33333"`, `"44"`, `"2"`, and `"3"`. Their run lengths are `2`, `2`, `5`, `2`, `1`, and `1`, so the output becomes `"225211"`.

> [!IMPORTANT]
> Refer to [`run_len_encode.c`](./Codes/run_len_encode.c) for the code used in this section.

The line `buf[out] = '0' + count;` works because the question guarantees that no character repeats more than 9 times in a row. That means each count fits in a single digit.

The buffer check is important. C strings must end with a null terminator, so the output buffer needs room for every digit we write plus one extra byte for `'\0'`. That is why the function checks `out + 1 >= buf_len` before writing the next digit. In C, strings are terminated by a null character, and string constants include that terminating null when stored in memory. Also, as we discussed earlier in today's tutorial, `strlen` counts the characters in the string but does not count that final null byte.

---

### A.5 2D Substring

> [!IMPORTANT]
> Refer to [`two_d.c`](./Codes/two_d.c) for the code used in this section.

This is really the same search pattern as the 1D string-search exercise in our [Week 3 Tutorial A](../Week%203/Week_3_Tutorial_A.md) notes: try every possible starting position, compare character by character, and stop as soon as something does not match. The only new idea is that instead of moving one step along a line, we move through the grid with a row step and a column step. Because the outer border of the grid is all `'\0'`, it behaves like a built-in stopping boundary, just like the null terminator at the end of an ordinary C string.

The helper function `match_direction` is the key idea. It starts at one cell and keeps comparing the next character of the word with the next cell in the chosen direction. Horizontal search uses step `(0, 1)`, vertical search uses `(1, 0)`, and diagonal search uses `(1, 1)`. If the whole word is consumed, the match succeeded.

**Extension:** *How to search in another diagonal?*

---

### A.6 C Maths Standard Library and `math.h`

The C standard library provides many mathematical functions in `<math.h>`. To use these functions, include the header and, on typical GCC/Unix-like setups, link against the math library:

```c
#include <math.h>
```

```bash
gcc -std=c23 my_math_program.c -lm -o program
```

You can inspect the header with `man math.h`, and inspect a specific function with commands such as `man 3 exp2`, `man 3 hypot`, or `man 3 log1p`.

> [!NOTE]
> A useful theme in `math.h` is that some functions exist for numerical correctness, not just convenience. `fma(x, y, z)` rounds once instead of twice, `hypot(x, y)` avoids undue overflow or underflow in the intermediate steps, and `log1p(x)` stays accurate when `x` is close to zero.

#### A.6.1 Function naming: `name`, `namef`, and `namel`

The usual pattern in `math.h` is that the unsuffixed function uses `double`, the `f` version uses `float`, and the `l` version uses `long double`. For example, `exp2`, `exp2f`, and `exp2l` all compute the same mathematical idea, but on different floating-point types.

> [!WARNING]
> The `f` and `l` are appended to the function name, not prepended. Also, in names like `exp2l()` or `log1pl()`, the `l` means `long double`, not `long`. Do not confuse that with `lround()` or `lrint()`, where the `l` is part of the function name because the return type is `long`.

**Many math functions have restricted domains. For example, `log1p(x)` has a finite real result only when `x > -1`, with `x = -1` being a pole, and `lgamma(x)` has poles at non-positive integers. Once you move beyond simple examples, always check the relevant man page for domain, range, and error behavior.**

#### A.6.2 `exp2(x)`

`exp2(x)` returns `2^x`, so it is a base-2 exponential function. If you specifically want powers of two, `exp2(x)` is often clearer than writing `pow(2.0, x)`.

```c
printf("%.1f\n", exp2(5.0));   // 32.0
printf("%.1f\n", exp2(3.0));   // 8.0
```

#### A.6.3 `fdim(x, y)`

`fdim(x, y)` returns the positive difference `max(x - y, 0)`. This is useful when you want to ask "how much bigger is `x` than `y`?" but never want a negative answer.

```c
printf("%.1f\n", fdim(5.5, 2.0));  // 3.5
printf("%.1f\n", fdim(2.0, 5.5));  // 0.0
```

For the meaning for `fdim`, I found a discussion names "What does the fdim acronym stand for?": <https://stackoverflow.com/questions/60227737/what-does-the-fdim-acronym-stand-for>

#### A.6.4 `fma(x, y, z)`

`fma` means fused multiply-add here.

`fma(x, y, z)` computes `x * y + z` as one floating-point operation and then rounds once according to the current rounding mode. That single-rounding behavior is the main reason `fma()` exists; it can be more accurate than doing the multiply and add separately. POSIX `math.h` also defines `FP_FAST_FMA`-style macros that indicate whether the `fma` family is generally as fast as, or faster than, separate multiply and add operations.

```c
printf("%.1f\n", fma(2.0, 3.0, 4.0));  // 10.0
```

#### A.6.5 `fmax(x, y)` and `fmin(x, y)`

`fmax()` returns the larger of two floating-point values, and `fmin()` returns the smaller. A useful detail is their NaN behavior: if one argument is NaN, the other argument is returned; if both arguments are NaN, the result is `NaN`.

This also connects nicely to the earlier warning about macro-based `MAX(a, b)`. `fmax()` and `fmin()` are real library functions with defined floating-point behavior, not text-substitution macros.

```c
printf("%.1f\n", fmax(3.2, 7.1));  // 7.1
printf("%.1f\n", fmin(3.2, 7.1));  // 3.2
```

#### A.6.6 `hypot(x, y)`

`hypot(x, y)` returns `sqrt(x*x + y*y)`, which is the Euclidean distance from the origin to `(x, y)` or the **hypotenuse** of a right triangle. The important detail is that it performs the calculation without undue overflow or underflow in the intermediate steps, so it is usually better than manually writing `sqrt(x*x + y*y)`.

```c
printf("%.1f\n", hypot(3.0, 4.0));  // 5.0
```

#### A.6.7 `lgamma(x)`

`lgamma(x)` returns the natural logarithm of the absolute value of the [Gamma function](https://en.wikipedia.org/wiki/Gamma_function). The related `signgam` value stores the sign of the Gamma function, and the `_r` variants return that sign through an argument instead of a global variable. Also, for nonnegative integers `m`, the Gamma function satisfies `Gamma(m + 1) = m!`, which is why `lgamma(5.0)` is `log(24)`.

```c
printf("%.6f\n", lgamma(5.0));  // about 3.178054
```

#### A.6.8 `lrint()` and `llrint()`

`lrint()` and `llrint()` round to the nearest integer using the current floating-point rounding direction. The difference between them is the return type: `lrint()` returns `long`, while `llrint()` returns `long long`. These functions are useful when your program deliberately cares about the active rounding mode.

#### A.6.9 `lround()` and `llround()`

`lround()` and `llround()` also round to an integer, but halfway cases are rounded away from zero and this rule does not depend on the current rounding direction. Again, the only difference between the two is the integer return type: `long` versus `long long`.

> [!CAUTION]
> `lrint()` and `lround()` are not interchangeable. `lrint()` follows the current rounding mode, while `lround()` always uses "nearest, halfway away from zero."

A small example for the `lround` family is:

```c
printf("%ld\n", lround(2.5));      // 3
printf("%lld\n", llround(-2.5));   // -3
```

##### A small rounding-mode demo

```c
#include <stdio.h>
#include <math.h>
#include <fenv.h>

#pragma STDC FENV_ACCESS ON

int main(void) {

    printf("lrint(2.9)  = %ld\n", lrint(2.9));    // 3
    printf("llrint(2.9) = %lld\n", llrint(2.9));  // 3
    printf("lround(2.9) = %ld\n", lround(2.9));   // 3
    printf("llround(2.9)= %lld\n", llround(2.9)); // 3

    fesetround(FE_DOWNWARD);

    printf("lrint(2.9)  = %ld\n", lrint(2.9));    // 2
    printf("llrint(2.9) = %lld\n", llrint(2.9));  // 2
    printf("lround(2.9) = %ld\n", lround(2.9));   // 3
    printf("llround(2.9)= %lld\n", llround(2.9)); // 3

    return 0;
}
```

This makes the difference concrete: with downward rounding active, `lrint()` and `llrint()` follow that rounding direction, while `lround()` and `llround()` still follow their own fixed rule.

#### A.6.9 `log1p(x)`

`log1p(x)` returns a value equivalent to `log(1 + x)`, but it is computed in a way that stays accurate when `x` is very close to zero. This is one of the classic examples of why the math library contains special-purpose functions: the special function can be more numerically stable than the obvious formula.

```c
printf("%.20f\n", log(1.0 + 1e-16));
printf("%.20f\n", log1p(1e-16));
```

The second line is the one you should trust for tiny values of `x`.

---

### A.7 Volume of A Sphere

> [!IMPORTANT]
> Refer to [`vsphere.c`](./Codes/vphere.c) for the code used in this section.

---

### A.8 Bits and Bitwise Operators

Computers store data as patterns of bits. In C, the core bitwise operators are `&`, `|`, `^`, `~`, `<<`, and `>>`. The bitwise operators work on integer types: `~` requires an integer operand, `&`, `^`, and `|` require integer operands and use the usual arithmetic conversions, and the shift operators also require integer operands.

> [!IMPORTANT]
> Refer to [`bits.c`](./Codes/bits.c) for the code used in this section for demonstration.

#### A.8.1 Reading a binary value

If we write the value `13` in 8 bits, we get:

```text
00001101
```

This means:

```text
0*128 + 0*64 + 0*32 + 0*16 + 1*8 + 1*4 + 0*2 + 1*1 = 13
```

For unsigned integers, each bit position represents a power of two. That is the basic mental model behind all bit manipulation in C.

#### A.8.2 Bitwise AND: `&`

The `&` operator keeps a result bit set only when both input bits are `1`. The C standard defines the result of `a & b` as the bitwise AND of the converted operands.

```c
unsigned char a = 12;   // 00001100
unsigned char b = 10;   // 00001010

unsigned char c = a & b; // 00001000 = 8
```

This is why `&` is commonly used to test whether a particular flag bit is on.

```c
if (flags & MASK) {
    /* MASK bit is set */
}
```

#### A.8.3 Bitwise OR: `|` and XOR: `^`

The `|` operator sets a result bit if at least one input bit is `1`. The `^` operator sets a result bit if exactly one input bit is `1`. These meanings are defined bit by bit in the standard after the usual arithmetic conversions.

```c
unsigned char a = 12;   // 00001100
unsigned char b = 10;   // 00001010

unsigned char x = a | b; // 00001110 = 14
unsigned char y = a ^ b; // 00000110 = 6
```

A simple way to remember them is that OR is often used to turn bits on, while XOR is often used to flip or toggle bits.

#### A.8.4 Bitwise NOT: `~`

The `~` operator flips every bit of its operand. A subtle point is that C performs integer promotions before applying `~`, so the result type is the promoted type, not necessarily the original small integer type.

```c
unsigned char a = 12;           // 00001100
unsigned int r = (~a) & 0xFFu;  // view only the low 8 bits

/* low 8 bits: 11110011 = 243 */
```

Masking with `0xFFu` is a simple way to show only the low 8 bits in an 8-bit classroom example.

#### A.8.5 Left shift: `<<` and right shift: `>>`

For `E1 << E2`, the bits of the left operand move left and the vacated positions are filled with zeros. For `E1 >> E2`, the bits move right. After integer promotions, the result type is the type of the promoted left operand. For unsigned values, left shift corresponds to multiplying by `2^E2` with wraparound, and right shift corresponds to dividing by `2^E2` with the fractional part discarded. For negative signed values, right shift is implementation-defined.

```c
unsigned char a = 12;   // 00001100

unsigned char left  = a << 1; // 00011000 = 24
unsigned char right = a >> 1; // 00000110 = 6
```

> [!WARNING]
> Be very careful with shifts. If the shift count is negative, or greater than or equal to the width of the promoted left operand, the behavior is undefined. Also, left-shifting signed values can become undefined, and right-shifting negative signed values is implementation-defined. For beginner bit manipulation, unsigned types are usually the safer choice.

#### A.8.6 Compound bitwise assignment

C also provides assignment forms such as `&=`, `|=`, `^=`, `<<=`, and `>>=`. These are the bitwise equivalents of operators like `+=`. For example, `flags |= MASK` means `flags = flags | MASK`.

```c
flags |= MASK;   // set some bits
flags &= MASK;   // keep only selected bits
flags ^= MASK;   // toggle selected bits
```

These forms are very common in systems code because they are short and readable once you know the pattern.

#### A.8.7 Common flag patterns

Bitwise operators become very useful when one integer stores many independent yes/no values. A common pattern is to create a mask with `1u << n`, meaning “the bit at position `n`”. Then OR is used to set a bit, AND with a complemented mask is used to clear a bit, XOR is used to toggle a bit, and AND is used to test a bit. Those patterns follow directly from the meanings of `|`, `~`, `^`, and `&`.

```c
enum {
    FLAG_READ  = 1u << 0,
    FLAG_WRITE = 1u << 1,
    FLAG_EXEC  = 1u << 2
};

unsigned int flags = 0;

flags |= FLAG_READ;          // set read
flags |= FLAG_WRITE;         // set write
flags &= ~FLAG_WRITE;        // clear write
flags ^= FLAG_EXEC;          // toggle execute

if (flags & FLAG_READ) {
    printf("read enabled\n");
}
```

This is the basic idea behind bit flags: one integer can store many independent on/off values.

---

### A.9 Exercise: Retro Game Inventory Manager

> [!IMPORTANT]
> Refer to [`bits.c`](./Codes/bits.c) for codes.
