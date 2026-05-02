## COMP2017 2026 S1 Week 9 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

[TOC]

---

### B.1 File Descriptor Table

We have learned about the file descriptor in week 5 when we talked about the file input/output. Today we are going to dig deeper and focus on its usages in process aspect.

The core idea is: *A process does not directly store open files, pipes, sockets, or terminals inside normal C variables. Instead, the operating system gives the process small integer handles called **file descriptors**. A process keeps these file descriptors in a **file descriptor table**.*

```text
file descriptor number  --->  kernel-managed open file / pipe / socket / terminal
```

![File Descriptor](../assets/img/File_table_and_inode_table.svg)

> By <a href="//commons.wikimedia.org/w/index.php?title=User:Qwertyus&amp;action=edit&amp;redlink=1" class="new" title="User:Qwertyus (page does not exist)">Qwertyus</a> - <span class="int-own-work" lang="en">Own work</span>, <a href="https://creativecommons.org/licenses/by-sa/4.0" title="Creative Commons Attribution-Share Alike 4.0">CC BY-SA 4.0</a>, <a href="https://commons.wikimedia.org/w/index.php?curid=38970813">Link</a>

So when we call:

```c
write(1, "hello\n", 6);
```

we are saying:

```text
write these bytes to whatever file descriptor 1 currently refers to
```

By convention, file descriptors `0`, `1`, and `2` are used for standard input, standard output, and standard error.

```text
0 = stdin
1 = stdout
2 = stderr
```

They are not magic numbers, but programs and the C library expect them to have these meanings.

---

### B.2

---

### B.3

---

### B.4

---

### B.5

---

### B.6
