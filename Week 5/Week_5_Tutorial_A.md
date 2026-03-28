## COMP2017 2026 S1 Week 5 Tutorial A

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 5 Tutorial A](#comp2017-2026-s1-week-5-tutorial-a)
  - [A.1 Files](#a1-files)
    - [A.1.1 Open the file: `fopen`](#a11-open-the-file-fopen)
      - [File modes](#file-modes)
      - [About the `x` specifier](#about-the-x-specifier)
    - [A.1.2 Read from the file: `fscanf`](#a12-read-from-the-file-fscanf)
    - [A.1.3 Write to the file: `fprintf`](#a13-write-to-the-file-fprintf)
    - [A.1.4 Close the file: `fclose`](#a14-close-the-file-fclose)
  - [Linux: “everything is a file”](#linux-everything-is-a-file)
  - [A.2 Exercise: Score to CSV](#a2-exercise-score-to-csv)
  - [A.3 Exercise: `cat`🐱](#a3-exercise-cat)
  - [A.4 Exercise: Replace Words](#a4-exercise-replace-words)
  - [A.5 Exercise: Word Count I](#a5-exercise-word-count-i)

---

### A.1 Files

In C, file I/O is stream-based. `fopen()` opens a file and returns a `FILE *`, `fscanf()` reads formatted text from that stream, `fprintf()` writes formatted text to that stream, and `fclose()` closes it. On Linux, the same file idea is reused for many kinds of resources, not just ordinary disk files, but also terminals, devices, pipes, and pseudo-filesystems such as `/proc`.

#### A.1.1 Open the file: `fopen`

`fopen(path, mode)` opens a file and returns a `FILE *` stream. If it fails, it returns `NULL`. The mode says what kind of access you want, such as read, write, or append.

```c
FILE *fp = fopen("data.txt", "r");
FILE *out = fopen("result.txt", "w");
```

Always check the result because it could fail:

```c
FILE *fp = fopen("data.txt", "r");
if (fp == NULL) {
    return 1;
}
```

##### File modes

- `"r"` means read an existing file.
- `"w"` means write to a file; create it if needed, or erase its old contents if it already exists.
- `"a"` means append to the end of a file; create it if needed.
- Adding `"+"` means the stream can both read and write. On Linux/POSIX systems, the `"b"` flag has no effect.

```c
fopen("in.txt", "r");    // read
fopen("out.txt", "w");   // write, truncate if exists
fopen("log.txt", "a");   // append
fopen("db.txt", "r+");   // read and write
```

##### About the `x` specifier

Adding `x` to `w` or `w+` makes the open exclusive: if the file already exists, `fopen()` fails instead of overwriting it. This is useful when you want to create a brand new file safely.

```c
FILE *fp = fopen("report.csv", "wx");
```

#### A.1.2 Read from the file: `fscanf`

`fscanf(stream, format, ...)` reads formatted input from a stream. It is basically the file version of `scanf()`. A useful habit is to check the return value, because it tells you how many items were successfully matched and assigned.

```c
int x;
fscanf(fp, "%d", &x);
```

```c
char name[20];
int mark;

if (fscanf(fp, "%19s %d", name, &mark) == 2) {
    /* read succeeded */
}
```

#### A.1.3 Write to the file: `fprintf`

`fprintf(stream, format, ...)` writes formatted output to a chosen stream. It is the file version of `printf()`. It writes according to the format string and returns the number of bytes printed, or a negative value on error.

```c
fprintf(fp, "Alice %d\n", 90);
```

```c
fprintf(stdout, "Done\n");
fprintf(stderr, "Bad input\n");
```

#### A.1.4 Close the file: `fclose`

When you are finished, call `fclose(fp)`. It flushes buffered output and closes the stream. After that, you **should not** use the same `FILE *` again. `fclose()` returns `0` on success and `EOF` on error.

```c
fclose(fp);
```

### Linux: “everything is a file”

Linux exposes many different resources through the same file-style interface. The Linux VFS provides a common filesystem interface to user programs; filesystem objects include things like regular files, directories, and FIFOs, and `/proc` is a pseudo-filesystem that exposes kernel data structures as files.

So in Linux, a “file” is not only a normal saved document on disk. It can also be a device, a terminal, a pipe, or a kernel-information entry. That is why the same basic ideas keep showing up: open, read, write, close.

Let's see an example:


```text
/home/student/notes.txt   ordinary file
/dev/null                 special file that discards data
/dev/tty                  your terminal
/proc/cpuinfo             CPU information from the kernel
```

`/dev/null` discards anything written to it, and reading from it immediately gives end-of-file. `/dev/tty` is the controlling terminal of the current process. `/proc/cpuinfo` is not a regular saved text file on disk; it is information provided by the kernel through the proc filesystem.

```bash
echo hello > /dev/null
cat /proc/cpuinfo | head
```

```c
FILE *fp = fopen("/dev/null", "w");
fprintf(fp, "this disappears\n");
fclose(fp);
```

```c
FILE *fp = fopen("/proc/cpuinfo", "r");
```

---

### A.2 Exercise: Score to CSV

> [!IMPORTANT]
> Refer to [`score_to_csv.c`](./Codes/score_to_csv.c) for the code used in this section.

The function `output_scores` takes an array of `struct batsman` and writes each element to a text file in CSV format. It first opens the file in write mode using `fopen(filename, "w")`. Then it loops over the array from index `0` to `no_batsmen - 1`. For each `batsman`, it writes the first name, last name, and score separated by commas. If the score is `0`, it writes the word `Duck` instead of the number. Finally, it closes the file with `fclose`.

Don't forget that each line ends with `\n`, because in a CSV file each record should be on its own line.

---

### A.3 Exercise: `cat`🐱

> [!IMPORTANT]
> Refer to [`cat.c`](./Codes/cat.c) for the code used in this section.

This program behaves like a simple version of Unix `cat`. If the user gives no filenames, it reads from `stdin` using `fgets` and prints each line to `stdout` using `fprintf`. If the user gives filenames, the program loops through each argument, opens the file with `fopen(..., "r")`, reads chunks using `fread`, and prints the bytes to `stdout` in order. After each file is finished, it closes the stream with `fclose`. The important idea is that `argc == 1` means there are no extra command-line arguments, so the program should read from standard input instead.

A few common mistakes are worth calling out. Do not start the file loop at `argv[0]`, because `argv[0]` is the program name. Do not add an extra newline after `fgets`, because the newline is usually already in the buffer. Also, do not write `while (!feof(fp))`; for file input, the loop should be controlled by the return value of `fread` or `fgets`.

Because the worksheet explicitly requires `fgets`, the no-argument branch is text-oriented rather than a perfect byte-for-byte clone of the system `cat`.

---

### A.4 Exercise: Replace Words

<iframe width="560" height="315" src="https://www.youtube.com/embed/dQw4w9WgXcQ?si=dRIKJoiGl0gu_cQ0" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>

> [!IMPORTANT]
> Refer to [`replace_word.c`](./Codes/replace_word.c) for the code and [`rick.txt`](./Codes/rick.txt) for the text used in this section.

The program scans the file character by character, stores one word at a time, replaces only exact whole-word matches, and copies every separator unchanged so the output keeps the original formatting.

Common mistakes worth noting are using `fscanf("%s", ...)` and accidentally losing spaces and newlines, forgetting to flush the final buffered word at EOF, calling `isalpha(ch)` without the `unsigned char` cast, and using the same file as both input and output.

> [!CAUTION]
> One limitation of this tutorial solution is that it treats a word as letters only, so it does not handle phrases, and something like `don't` is split around the apostrophe.

---

### A.5 Exercise: Word Count I

> [!IMPORTANT]
> Refer to [`wc.c`](./Codes/wc.c) for the code used in this section.

This solution uses `fread` to process the input in blocks instead of one character at a time. Calling `fread(buffer, 1, sizeof buffer, fp)` makes the return value equal to the number of bytes read, so byte counting is easy: add `nread` to the total each time. Then loop over the bytes in the buffer. Count lines by checking for `'\n'`. Count words with an `in_word` flag: whitespace sets `in_word` to `0`, and a non-whitespace byte starts a new word only if `in_word` was previously `0`. This prevents letters in the same word from being counted multiple times.

The two most common mistakes in a `fread` version are forgetting that a word may continue across buffer boundaries, and writing `isspace(ch)` on a plain signed `char`. The safe form is `isspace((unsigned char) ch)`, because values not representable as `unsigned char` or `EOF` are undefined for the classification functions. Also, after the `fread` loop ends, `ferror(fp)` is the right way to detect a real read failure instead of normal EOF.

> [!NOTE]
> I used `fread` for the core algorithm and `printf` for the short count lines. If you want to use `fwrite` too, the usual tweak is to build the output line with `snprintf` and then `fwrite` that string to `stdout`.
