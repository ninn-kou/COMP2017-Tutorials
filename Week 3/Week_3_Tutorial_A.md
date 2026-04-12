## COMP2017 2026 S1 Week 3 Tutorial A

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 3 Tutorial A](#comp2017-2026-s1-week-3-tutorial-a)
  - [A.1 Characters and ASCII Codes](#a1-characters-and-ascii-codes)
  - [A.2 String in C](#a2-string-in-c)
    - [A.2.1 What a C string Actually Is?](#a21-what-a-c-string-actually-is)
    - [A.2.2 The Two Most Important String Ideas](#a22-the-two-most-important-string-ideas)
  - [A.3 Command Line Arguments](#a3-command-line-arguments)
    - [A.3.1 Exercise: `cla.c`](#a31-exercise-clac)
  - [A.4 String Manipulation](#a4-string-manipulation)
    - [A.4.1 Exercise: Your Own `atoi`](#a41-exercise-your-own-atoi)
      - [Step 1: The Minimum Working Code](#step-1-the-minimum-working-code)
      - [Step 2: Handle Letters](#step-2-handle-letters)
      - [Step 3: Check the Sign](#step-3-check-the-sign)
      - [Step 4: Skip Leading Spaces](#step-4-skip-leading-spaces)
    - [A.4.2 Exercise: Your Own `reverse`](#a42-exercise-your-own-reverse)
      - [Why `fgets` and Not `scanf`?](#why-fgets-and-not-scanf)
      - [How a Line Looks in Memory](#how-a-line-looks-in-memory)
      - [Our Main Idea](#our-main-idea)
      - [Step by Step Attempt](#step-by-step-attempt)
    - [A.4.3 Extension Exercise: Your Own `itoa`](#a43-extension-exercise-your-own-itoa)
    - [A.4.4 Exercise: Search A Word in A String](#a44-exercise-search-a-word-in-a-string)
      - [Step 1: `string_compare()`](#step-1-string_compare)
      - [Step 2: String Length](#step-2-string-length)
      - [Step 3: Contains Word](#step-3-contains-word)
      - [Step 4: Our Algorithm](#step-4-our-algorithm)

---

### A.1 Characters and ASCII Codes

**ASCII is a standard table that gives a numeric code to each character.** For example, the letter `'A'`, the digit `'0'`, a space, and many other characters each *have a number behind them*.

![ASCII Table](../assets/img/ASCII_Table_(suitable_for_printing).svg)

For example:

- `'0'` = 48
- `'1'` = 49
- `'9'` = 57
- `'A'` = 65
- `'Z'` = 90
- `'a'` = 97
- `'z'` = 122
- `' '` = 32
- `'\0'` = 0

So when you write this in C:

```c
char c = 'A';
```

the computer actually stores a numeric code for that character.

In C, the type `char` is really a small integer type used to store character codes. That is why characters can be compared and even used in arithmetic expressions.

For example:

```c
printf("%d\n", 'A');   // prints 65 on ASCII-based systems
printf("%d\n", '0');   // prints 48
printf("%d\n", '9');   // prints 57
```

> [!TIP]
> `'0'` and `0` are not the same thing.

- `'0'` is the character zero
- `0` is the integer zero
- `'\0'` is the null character, used to mark the end of a string

This matters a lot in strings.

For example, the string `"123"` is stored in memory like this:

- `'1'`
- `'2'`
- `'3'`
- `'\0'`

That last `'\0'` tells C where the string ends.

**C does not strictly require ASCII on every machine, but on modern systems it is almost always ASCII or an ASCII-compatible encoding such as UTF-8.** More importantly, C guarantees that the digit characters `'0'` through `'9'` are consecutive, so the trick `s[i] - '0'` is valid in standard C.

---

### A.2 String in C

#### A.2.1 What a C string Actually Is?

A C string is not a special built-in "string object." It is just a sequence of `char` values that ends with a null byte, written as `'\0'`. A string literal like `"cat"` has a zero byte appended to it by the compiler.

```c
char s[] = "cat";
```

That is stored like this in memory:

```c
'c'   'a'   't'   '\0'
```

That last `'\0'` is what tells C where the string ends.

This is why string functions keep scanning characters until they reach `'\0'`.

> [!WARNING]
> The string literal itself should be treated as read-only. The C standard says that attempting to modify the array for a string literal is undefined behavior.
>
> For your information:
> <https://stackoverflow.com/questions/1455970/cannot-modify-c-string>

Good beginner rule:

```c
char s[] = "cat";        // okay to modify s
const char *p = "cat";   // point at a literal, treat as read-only
```

#### A.2.2 The Two Most Important String Ideas

First, a string ends at `'\0'`, not at the size of the array.

Second, the array holding the string must have enough room for the characters **and** the final `'\0'`.

So this is correct:

```c
char name[6] = "Alice";
```

because `"Alice"` needs 5 letters plus 1 null terminator.

This is wrong:

```c
char name[5] = "Alice";
```

because there is no room for `'\0'`.

You don't need to explicitly declare the string size if you create them when initialize string like this.

```c
char name[] = "Alice";
```

> [!NOTE]
> A C string is just a `char` array that ends with `'\0'`, and `<string.h>` gives you functions to measure it, compare it, copy it, join it, and search it. We will cover more about C Standard Library in Tutorial B.

---

### A.3 Command Line Arguments

Command line arguments are the words you type after the program name when you run a program in the terminal.

For example:

```bash
./cla Echo 419 to Cortana, come in
```

Here:

- `./cla` is the program
- everything after it is a command line argument

So the program receives these arguments:

```text
Echo
419
to
Cortana,
come
in
```

In C, you usually access them with this form of `main`:

```c
int main(int argc, char *argv[])
```

You may also see:

```c
int main(int argc, char **argv)
```


`argc` means "argument count". It tells you how many arguments there are.

`argv` means "argument vector". It is a list of strings.

The important beginner idea is this:

- one C string is a `char *`
- command line input is many strings
- so `argv` is a list of `char *`
- that is why its type is `char **`

A nice way to picture it is this:

```text
argv
 |
 +--> argv[0] --> "./cla"
 +--> argv[1] --> "Echo"
 +--> argv[2] --> "419"
 +--> argv[3] --> "to"
 +--> argv[4] --> "Cortana,"
 +--> argv[5] --> "come"
 +--> argv[6] --> "in"
```

So if the command is:

```bash
./cla Echo 419 to Cortana, come in
```

then:

```c
argc == 7
argv[0] == "./cla"
argv[1] == "Echo"
argv[2] == "419"
argv[3] == "to"
argv[4] == "Cortana,"
argv[5] == "come"
argv[6] == "in"
```

```c
argv[1] == "Echo"
argv[1][0] == 'E'
argv[1][1] == 'c'
```

A very important detail: `argv[0]` is usually the program name. That means the user's actual arguments start at `argv[1]`.

That is why programs often loop like this:

```c
for (i = 1; i < argc; i++) {
    ...
}
```

#### A.3.1 Exercise: `cla.c`

> [!IMPORTANT]
> Refer to [`cla.c`](./Codes/cla.c) for the code used in this section.

The program should print all arguments given to it, excluding the program name, with spaces between them.

We start at `i = 1` because `argv[0]` is the program name and we do not want to print it.

For each argument, we print:

```c
printf("%s", argv[i]);
```

We use `%s` because each `argv[i]` is a string.

Then we print a space after it, except for the last one:

```c
if (i < argc - 1) {
    printf(" ");
}
```

That avoids an extra trailing space at the end.

Finally, we print a newline.

Example run:

```bash
$ ./cla Echo 419 to Cortana, come in
Echo 419 to Cortana, come in
```

**Here are some common mistakes to point out.**

- The first is printing `argv[0]` by accident. That prints the program name too, which the exercise does not want.
- The second is using `%c` instead of `%s`. `argv[i]` is a string, not a single character, so `%s` is correct.
- The third is forgetting that the shell splits arguments on spaces. So:

```bash
./cla hello world
```

gives two arguments:

```text
"hello"
"world"
```

But:

```bash
./cla "hello world"
```

gives one argument:

```text
"hello world"
```

---

### A.4 String Manipulation

#### A.4.1 Exercise: Your Own `atoi`

> [!IMPORTANT]
> Refer to [`my_atoi.c`](./Codes/my_atoi.c) for the code used in this section.

Let's first look at a simple case and try to convert the string `2017` into an integer together.

##### Step 1: The Minimum Working Code

Starting from the beginning of the string, we read the characters one by one. The first character is `2`, which represents two thousands, followed by `0` for zero hundreds, `1` for one ten, and `7` for seven. Each digit before the next one is 10 times larger in value, so one idea is to multiply the current number by 10 each time we move to the next digit.

We know that:

```
2017 = 2 * 1000 + 0 * 100 + 1 * 10 + 7 * 1
```

So we can use an integer variable to store the result as we build the number step by step.

For `"2017"`:

- start: `n = 0`
- read `'2'` → digit = 2 → `n = 0 * 10 + 2 = 2`
- read `'0'` → digit = 0 → `n = 2 * 10 + 0 = 20`
- read `'1'` → digit = 1 → `n = 20 * 10 + 1 = 201`
- read `'7'` → digit = 7 → `n = 201 * 10 + 7 = 2017`

However, how can we convert each character into a digit? By considering how characters are encoded in the system, we can use `s[i] - '0'`, where `s` is the string and `i` is the current index. This gives us the numeric offset from the character `'0'`, which corresponds exactly to the digit value.

Therefore, we can write:

```c
int atoi(const char s[]) {
    int i = 0;
    int n = 0;

    /* Read digits and build the number */
    while (s[i] != '\0') {
        n = n * 10 + (s[i] - '0');
        i++;
    }

    return n;
}
```

##### Step 2: Handle Letters

What happens if we try the following in the `main` function?

```c
printf("%d\n", atoi("a"));
printf("%d\n", atoi("abc"));
```

**Try it yourself.**

```
printf("%d\n", atoi("abc"));    // 5451
5451 = 49 * 100 + 50 * 10 + 51
```

This happens because `a`, `b`, and `c` each have integer values in the character encoding table. However, this is not the behavior we expect. We need to check whether the character is a digit.

Therefore, we can update the `while` condition to ensure we only process digits:

```c
int atoi(const char s[]) {
    int i = 0;
    int n = 0;

    /* Read digits and build the number */
    while (s[i] >= '0' && s[i] <= '9') {
        n = n * 10 + (s[i] - '0');
        i++;
    }

    return n;
}
```

##### Step 3: Check the Sign

What happens if the string starts with a sign (`+` or `-`)? In this case, the `while` condition is not satisfied, so the entire loop would be skipped. Therefore, we need to add functionality to handle the sign.

We can add a preprocessing step to check for a sign and move the index forward if one is found.

```c
int atoi(const char s[]) {
    int i = 0;
    int sign = 1;
    int n = 0;

    /* Check sign */
    if (s[i] == '-') {
        sign = -1;
        i++;
    } else if (s[i] == '+') {
        i++;
    }

    /* Read digits and build the number */
    while (s[i] >= '0' && s[i] <= '9') {
        n = n * 10 + (s[i] - '0');
        i++;
    }

    return sign * n;
}
```

##### Step 4: Skip Leading Spaces

Our code looks good so far, but it still cannot handle some "special" characters such as leading whitespace. Therefore, we need one more improvement.

```c
int atoi(const char s[]) {
    int i = 0;
    int sign = 1;
    int n = 0;

    /* Skip leading whitespace */
    while (s[i] == ' ' || s[i] == '\t' || s[i] == '\n') {
        i++;
    }

    /* Check sign */
    if (s[i] == '-') {
        sign = -1;
        i++;
    } else if (s[i] == '+') {
        i++;
    }

    /* Read digits and build the number */
    while (s[i] >= '0' && s[i] <= '9') {
        n = n * 10 + (s[i] - '0');
        i++;
    }

    return sign * n;
}
```

#### A.4.2 Exercise: Your Own `reverse`

> [!IMPORTANT]
> Refer to [`reverse.c`](./Codes/reverse.c) for the code used in this section.

The task is to read input **one whole line at a time**, reverse the characters in that line, and print the reversed line. The important detail is that this is a **character-by-character** reversal, not a word-by-word reversal.

For example,

```text
abc 123    // Input

321 cba    // Output
```

##### Why `fgets` and Not `scanf`?

`scanf("%s", line)` stops reading at the first whitespace character. In C, whitespace includes:

- space `' '`
- tab `'\t'`
- newline `'\n'`

That is what `isspace` is about. The function `isspace` from `<ctype.h>` checks whether a character is whitespace.

For example:

```c
isspace(' ')   // true
isspace('\t')  // true
isspace('\n')  // true
```

So if the input is:

```text
abc 123
```

then `scanf("%s", line)` reads only:

```text
abc
```

and stops before the space.

But `fgets` reads the **whole line**, including spaces, and usually also keeps the newline character `'\n'`. That is exactly what we want.

##### How a Line Looks in Memory

Suppose the input line is:

```text
abc 123
```

After `fgets`, the array looks like this:

```c
'a'  'b'  'c'  ' '  '1'  '2'  '3'  '\n'  '\0'
```

Two special characters matter here:

- `'\n'` is the newline at the end of the line
- `'\0'` is the null terminator that marks the end of the C string

For this exercise, we usually want to reverse the visible part:

```text
abc 123
```

but keep the newline `'\n'` at the end.

Otherwise, if we reverse `'\n'` too, it moves to the front and the output formatting becomes strange.

##### Our Main Idea

The easiest way to reverse a string in place is to use two positions:

- one starting at the front
- one starting at the back

Then swap those characters, move inward, and repeat.

For `"abc 123"`:

- swap `'a'` and `'3'`
- swap `'b'` and `'2'`
- swap `'c'` and `'1'`

The middle space stays where it ends up naturally.

##### Step by Step Attempt

1. Read one line using `fgets`.
2. Find the length of the string.
3. If the last real character is `'\n'`, do not reverse that character.
4. Set:
   - `i` to the start of the line
   - `j` to the last character to reverse
5. Swap `s[i]` and `s[j]`.
6. Move `i` forward and `j` backward.
7. Repeat until `i >= j`.
8. Print the line.
9. Keep going until `fgets` reaches end-of-file.

#### A.4.3 Extension Exercise: Your Own `itoa`

> [!IMPORTANT]
> Refer to [`my_itoa.c`](./Codes/my_itoa.c) for the code used in this section.

`itoa` is the mirror image of `atoi`, and it convert the integer to a string.

> [!NOTE]
> `itoa` is commonly taught and sometimes provided by compilers, but it is **not** a standard C library function. That is why in teaching code it is better to write your own version, such as `my_itoa`.

For `atoi`, you convert a digit character into a number like this:

```c
digit = s[i] - '0';
```

For `itoa`, you convert a number into a digit character like this:

```c
ch = digit + '0';
```

`itoa` is a little trickier than `atoi` because the easiest way to get the digits of a number is from right to left.

Take `1234`:

- `1234 % 10` gives `4`
- `1234 / 10` gives `123`
- `123 % 10` gives `3`
- `12 % 10` gives `2`
- `1 % 10` gives `1`

So we have a step-by-step attempt:

1. Remember whether the number is negative.
2. Work with its positive magnitude.
3. Take the last digit using `% 10`.
4. Convert that digit into a character using `+ '0'`.
5. Store that character in the string.
6. Remove the last digit using `/ 10`.
7. Repeat until there are no digits left.
8. Add `'-'` if the original number was negative.
9. Add the string terminator `'\0'`.
10. Reverse the string.

A very important beginner point is that `itoa` cannot just "return a string" the same easy way `atoi` returns an `int`. In C, strings are arrays of characters, so the function needs a character array to write into.

#### A.4.4 Exercise: Search A Word in A String

**This exercise combines three earlier ideas very neatly: command-line arguments, `fgets`, and string searching.**

When you run

```bash
./wordsearch Sushi < restaurants.txt
```

the shell does two things for your program:
- `argv[1]` becomes `"Sushi"`, so that is the word you are looking for.
- `stdin` is no longer the keyboard. Because of `< restaurants.txt`, standard input now comes from the file. That means you do not need `fopen` for this version. You can just keep reading from `stdin` with `fgets`.

So we have a basic plan looks like:
1. Read the search word from the command line.
2. Read one line at a time with `fgets`.
3. Remove the trailing newline `'\n'` from the line.
4. Try every possible starting position in the line.
5. At each position, copy out a chunk the same length as the search word.
6. Use `string_compare` to compare that chunk with the search word.
7. If they match, print the line with `Found: `.

##### Step 1: `string_compare()`

```c
int string_compare(const char s1[], const char s2[]) {
    int i = 0;

    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') {
            return 0;   /* equal */
        }
        i++;
    }

    return s1[i] - s2[i];
}
```

##### Step 2: String Length

```c
int string_length(const char s[]) {
    int i = 0;

    while (s[i] != '\0') {
        i++;
    }

    return i;
}
```

##### Step 3: Contains Word

Here is the idea behind `contains_word`.

Suppose the line is:

```text
157 Redfern St, Sushi Topia
```

and the word is:

```text
Sushi
```

The program tries the word starting at every position in the line.

First it checks something like:

```text
157 R
```

Then:

```text
57 Re
```

Then:

```text
7 Red
```

and so on.

Eventually it reaches:

```text
Sushi
```

At that point `string_compare(part, word)` says they are equal, so the line should be printed.

That is the main search pattern: "try every starting position."

```c
void copy_part(const char src[], int start, int len, char dest[]) {
    int i;

    for (i = 0; i < len && src[start + i] != '\0'; i++) {
        dest[i] = src[start + i];
    }

    dest[i] = '\0';
}

int contains_word(const char line[], const char word[]) {
    int i;
    int word_len = string_length(word);
    char part[MAX_LINE];

    if (word_len == 0) {
        return 1;
    }

    if (word_len >= MAX_LINE) {
        return 0;
    }

    for (i = 0; line[i] != '\0'; i++) {
        copy_part(line, i, word_len, part);

        if (string_compare(part, word) == 0) {
            return 1;
        }
    }

    return 0;
}
```

##### Step 4: Our Algorithm

> [!IMPORTANT]
> Refer to [`wordsearch.c`](./Codes/wordsearch.c) for the code used in this section.

```c
int main(int argc, char *argv[]) {
    char line[MAX_LINE];
    char *word;

    if (argc != 2) {
        printf("Usage: %s word\n", argv[0]);
        return 1;
    }

    word = argv[1];

    while (fgets(line, sizeof line, stdin) != NULL) {
        trim_newline(line);

        if (contains_word(line, word)) {
            printf("Found: %s\n", line);
        }
    }

    return 0;
}

```

`trim_newline(line)` removes the `'\n'` that `fgets` usually keeps. That makes printing easier. Without trimming, this:

```c
printf("Found: %s\n", line);
```

would usually print an extra blank line, because `line` already ends with `'\n'`.

```c
void trim_newline(char s[]) {
    int i = 0;

    while (s[i] != '\0') {
        if (s[i] == '\n') {
            s[i] = '\0';
            return;
        }
        i++;
    }
}
```

A few parts are especially important.

`fgets(line, sizeof line, stdin)` reads a whole line, including spaces. That is why it works for restaurant addresses and names like `"Sushi Topia"`. If you used `scanf("%s", line)`, it would stop at the first space and break the exercise.

`copy_part` builds a small temporary string from the line so that `string_compare` can compare whole strings. That is the key trick that lets you reuse last week’s comparison function.

> [!WARNING]
> **A common mistake is using `==` directly on strings**, like this:
>
> ```c
> if (part == word)
> ```
>
> That does not compare the contents of the strings. **It only compares addresses.** In C, strings must be compared character by character, which is why you need `string_compare`.

Another classic mistake is forgetting the null terminator in the temporary substring:

```c
dest[i] = '\0';
```

Without that line, `part` is not a proper C string, and `string_compare` may keep reading garbage beyond the intended end.
