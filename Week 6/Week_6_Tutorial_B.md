## COMP2017 2026 S1 Week 6 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

[TOC]

---

### B.1 Device I/O, File Descriptors, and System Calls

#### B.1.1 Two-layers I/O Model

The lower layer is the Unix/POSIX file-descriptor interface, which uses calls like `open()`, `read()`, `write()`, `close()`, and `fcntl()`. The higher layer is the C standard I/O library, which gives you buffered `FILE *` streams and functions like `fopen()`, `fread()`, `fwrite()`, `fprintf()`, and `fclose()`. In ordinary C programs, system calls are usually reached through libc wrapper functions, so calling `read()` or `open()` looks like a normal function call even though it is entering the kernel. A useful man-page rule is that the syscall interfaces here live in section 2, while the C library stream interfaces live in section 3.

A slightly more precise version of “everything is a file” is: Unix exposes many resources through the same file or stream model. The `stdio` manual says a stream is associated with an external file, which may be a physical device, and `open()` can also open device special files. That is why ordinary disk files, terminals, pipes, and many devices can all be handled with the same core ideas: open, read, write, close.

#### B.1.2 File Descriptors

A **file descriptor** is the low-level handle for an open file-like object. It is just a small nonnegative integer that indexes an entry in the process’s table of open file descriptors. One important correction to the slide: `open()` returns the **lowest-numbered file descriptor not currently open** for the process, not the “next largest” number. So if `0`, `1`, `2`, `4`, and `6` are already open, the next successful `open()` returns `3`. `open()` also creates an **open file description**, which is where the file offset and file status flags are stored.

At program startup, three standard streams already exist: `stdin`, `stdout`, and `stderr`. Their underlying file descriptors are `0`, `1`, and `2`, and the macros `STDIN_FILENO`, `STDOUT_FILENO`, and `STDERR_FILENO` are defined with those values in `<unistd.h>`. The `stdin(3)` page also says that `stdin`, `stdout`, and `stderr` are macros of type “pointer to `FILE`”, so they belong to the high-level stream layer even though they sit on top of those familiar descriptor numbers.

- `int fd` is the low-level kernel-facing handle.
- `FILE *fp` is the higher-level C library stream.

The `stdio` library is a buffered stream interface: it maps I/O into logical streams and hides the physical details of the device or file underneath. The `stdin(3)` page says `FILE`s are buffering wrappers around Unix file descriptors, and `fileno()` is the bridge back down: it returns the integer file descriptor used to implement a stream.

That is why `stdio` is usually easier for normal text or formatted I/O, while raw descriptor I/O is used when you need tighter control over flags and behavior, especially for terminals, pipes, sockets, and devices. In particular, `open()` supports file status flags such as `O_NONBLOCK`, and `fcntl()` exposes operations like `F_GETFL` and `F_SETFL` for reading and changing those status flags later.

#### B.1.3 Opening Files: `open()` and `fopen()`

At the low level, you open with `open()` and get back an `int` file descriptor. At the high level, you open with `fopen()` and get back a `FILE *` stream. The man pages document `open()` in `<fcntl.h>`, `read()`/`write()`/`close()` in `<unistd.h>`, and `fopen()` in `<stdio.h>`. `fopen()` associates a stream with the file, while `open()` gives you the raw descriptor directly.

```c
/* low-level */
int fd = open("file.txt", O_RDONLY);

/* high-level */
FILE *fp = fopen("file.txt", "r");
```

The two layers line up closely. `fopen()` mode strings correspond to `open()` flags: `"r"` behaves like `O_RDONLY`, `"w"` like `O_WRONLY | O_CREAT | O_TRUNC`, `"a"` like `O_WRONLY | O_CREAT | O_APPEND`, and the `+` modes map to `O_RDWR` variants. That makes `fopen()` feel less “magical”: it is a friendlier wrapper around the same underlying idea.

> [!NOTE]
> `open()` returns the **lowest-numbered unused** descriptor, not the "next biggest" number.

#### B.1.4 Reading: `read()` and `fread()`

At the descriptor layer, `read(fd, buf, count)` reads **up to** `count` bytes and returns the number of bytes actually read, `0` for end-of-file, or `-1` on error. A very important syscall detail is that a successful `read()` may return **fewer** bytes than requested, especially near EOF or when reading from things like pipes or terminals.

At the stream layer, `fread(ptr, size, n, fp)` reads `n` items, each `size` bytes long, and returns the number of **items** read. That equals a byte count only when `size == 1`. So in your slide example, `fread(buf, sizeof(char), 100, fp)` behaves like a byte-oriented read because `sizeof(char)` is `1`, but in general `fread()` and `read()` are not returning the same kind of count. 

```c
/* low-level */
ssize_t n = read(fd, buf, 100);

/* high-level */
size_t n = fread(buf, 1, 100, fp);
```

#### B.1.5 Writing: `write()` and `fwrite()`

At the descriptor layer, `write(fd, buf, count)` writes up to `count` bytes and returns the number of bytes actually written, or `-1` on error. Just like `read()`, `write()` may complete only part of the job, so serious low-level code often needs a loop that keeps writing until all bytes are sent. This matters a lot for pipes, sockets, and some device-style I/O. The Linux `write(2)` man page says this explicitly: “a successful `write()` may transfer fewer than `count` bytes”, and in that case the caller should call `write()` again for the remaining bytes.

At the stream layer, `fwrite(ptr, size, n, fp)` returns the number of **items** written, not necessarily bytes unless `size == 1`. Because `stdio` is buffered, the bytes may first sit in a user-space stream buffer instead of immediately reaching the underlying descriptor. That is why `fflush(fp)` exists: it forces buffered output for that stream through its underlying write function.

```c
/* low-level */
ssize_t m = write(fd, buf, 100);

/* high-level */
size_t m = fwrite(buf, 1, 100, fp);
fflush(fp);
```

#### B.1.6 Closing: `close()` and `fclose()`

At the descriptor layer, `close(fd)` closes the file descriptor so it no longer refers to the file and may later be reused. It returns `0` on success and `-1` on error. If it is the last descriptor referring to the underlying open file description, the associated resources are freed.

At the stream layer, `fclose(fp)` is stronger than just “close the stream”: it first flushes buffered output for that stream and then closes the underlying file descriptor. So that you do **not** need to say “`fwrite(...); fflush(fp); fclose(fp);`” as the normal closing pattern. `fflush()` is useful when you want data pushed out **before** you are done, but `fclose()` already performs that flush-and-close behavior.

```c
/* low-level */
close(fd);

/* high-level */
fclose(fp);
```

#### B.1.7 `fcntl()`, Flags, and Device Control

This is where raw file-descriptor I/O starts to matter more. `fcntl()` is the general “manipulate file descriptor” call, and one of its jobs is getting and setting file status flags with `F_GETFL` and `F_SETFL`. `open()` also accepts flags such as `O_NONBLOCK`, and the manual says that when possible, a descriptor opened in nonblocking mode will not cause the caller to wait in `open()` or later I/O operations. On Linux, though, `O_NONBLOCK` has no real effect for regular files and block devices, so it matters most for terminals, pipes, FIFOs, sockets, and similar resources.

*We will then see an example in the nonblocking-stdin exercise.*

---

### B.2 Exercise: `cat`🐱🐱

> [!IMPORTANT]
> Refer to [`cat.c`](./Codes/cat.c) for the code used in this section.

`open()` is declared in `<fcntl.h>`, the "**file control options**" header file. while `read()`, `write()`, `close()`, and the standard descriptor macros such as `STDIN_FILENO` and `STDOUT_FILENO` come from `<unistd.h>`, the "**standard symbolic constants and types**" header file. `open()` returns a small nonnegative file descriptor, `read()` reads up to the requested byte count and returns `0` at end-of-file, `write()` may write fewer bytes than you asked for, and `close()` closes the descriptor. To easily get the length of string, we also need the `<string.h>`. And `<errno.h>` for error numbers as flags.

```c
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
```

`main` has two cases. If `argc == 1`, there were no filenames, so the program reads from standard input. On Unix systems, standard input, output, and error are the descriptors `0`, `1`, and `2`, and the symbolic names `STDIN_FILENO`, `STDOUT_FILENO`, and `STDERR_FILENO` are provided in `<unistd.h>`. If filenames are given, the program opens each file with `open(argv[i], O_RDONLY)`. `open()` returns a file descriptor on success, and the returned descriptor is the lowest-numbered descriptor not currently open in the process. After that, the descriptor can be used with `read()`, `write()`, and `close()`.

The helper `copy_fd` does the real copying. It reads a block of bytes into `buffer` using `read()`. If `read()` returns `0`, that means end-of-file. If it returns a positive number, that many bytes were placed in the buffer and must be written to standard output. `read()` is allowed to return fewer bytes than requested, especially near EOF or when reading from things like pipes or terminals, so the code uses the actual return value `nread`, not the full buffer size.

The helper `write_all` exists because `write()` may write only part of the buffer. That is not necessarily an error, so we keep calling `write()` until the whole chunk has been sent. Without this inner loop, some output could be lost. `write()` can also fail with `EINTR` if interrupted before writing any bytes, so the code retries in that case.

At the end of each file, `close(fd)` releases the descriptor so it no longer refers to the file and may be reused later. `close()` returns `0` on success and `-1` on error.

---

### B.3 Non blocking STDIN

> [!IMPORTANT]
> Refer to [`pulse.c`](./Codes/pulse.c) for the code used in this section.

This program makes `stdin` nonblocking using `fcntl`. Each loop iteration tries to `read()` from standard input. If no input is available, `read()` fails with `EAGAIN`/`EWOULDBLOCK`, so the program prints how long it has been waiting, sleeps for one second, and tries again. If input is read, the program prints it and resets the counter. If the line is `"exit"`, the program terminates.

#### B.3.1 How It Works

`STDIN_FILENO` is the standard input file descriptor from `<unistd.h>`, and `F_GETFL` / `F_SETFL` let you read and change the file status flags. `F_SETFL` can change `O_NONBLOCK`, and POSIX recommends a read-modify-write pattern, which is why the code first gets the old flags and then sets `old_flags | O_NONBLOCK` instead of overwriting everything.

`read()` returns a positive number when bytes were read, `0` on end-of-file, and `-1` on error. For a nonblocking file descriptor, if no input is available, `read()` returns `-1` and sets `errno` to `EAGAIN`; portable code often checks `EWOULDBLOCK` too.

A useful terminal detail: on a normal terminal, input is usually in **canonical mode** by default, so input becomes available line by line, usually after you press Enter. That means this program is nonblocking, but it is still line-based unless you also change the terminal mode with `termios`.

The restore step at the end is worth keeping. After `fork()`, the child and parent descriptors refer to the same open file description and therefore share file status flags. Restoring the original flags avoids leaving the invoking shell’s stdin in nonblocking mode after your program exits.

---

### B.4 Function Pointers

---

### B.5 Exercise: Sorting with `qsort`
