## COMP2017 Systems Programming Tutorial Notes

<big><b>Semester 1, 2026</b></big>

---

Welcome! This repository contains notes and examples for my COMP2017 tutorial classes. They are intended to help you review key concepts, practise further, and revise after class.

**Please note that if anything in these notes differs from the lecture slides, the lecture slides take precedence.**

If you have questions about the tutorial content, please feel free to get in touch by email or ask me in tutorials.

---

### Tutor

<table><tbody>
  <tr><td><b>Tutor</b></td><td><b>Hao Ren</b></td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au"><b>hao.ren@sydney.edu.au</b></a></td></tr>
</tbody></table>

#### Tutorial Schedule

| Time        | Monday                                 | Wednesday | Thursday |
|-------------|----------------------------------------|-----------|----------|
| 10:00-11:00 |                                        |           | **B/02** |
| 11:00-12:00 |                                        | **B/47**  | **B/03** |
| 12:00-13:00 | **<del>A/14</del>**<sup>&dagger;</sup> | **B/13**  |          |
| 13:00-14:00 | **<del>A/15</del>**<sup>&dagger;</sup> | **B/14**  |          |
| 14:00-15:00 | **<del>A/16</del>**<sup>&dagger;</sup> |           |          |
| 15:00-16:00 | **A/17**                               |           |          |
| 16:00-17:00 | **A/18**                               | **A/39**  |          |
| 17:00-18:00 | **A/19**                               | **A/38**  |          |

<sup>&dagger;</sup> *I taught A/14, A/15, and A/16 in Weeks 2 and 3 only.*

---

### Table of Contents

<table>
  <thead>
    <tr>
      <th>Weeks</th>
      <th>Tutorials</th>
      <th>Ed Lessons</th>
      <th>Contents</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td rowspan="2">Week 2</td>
      <td><a href="./Week%202/Week_2_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99432/slides/682595">Week 1 All</a></td>
      <td>
        <ul>
          <li>Compiler</li>
          <li>C Documents</li>
          <li>C Types</li>
          <li>Simple Pointers</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td><a href="./Week%202/Week_2_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99433/slides/682616">Week 2 Part 1</a></td>
      <td>
        <ul>
          <li>Intro to Linux</li>
          <li>Command Line</li>
          <li>PATH</li>
          <li>Common Linux Commands</li>
          <li><code>sizeof</code></li>
          <li>Pointer Arithmetic</li>
          <li>Basic I/O</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td rowspan="2">Week 3</td>
      <td><a href="./Week%203/Week_3_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99433/slides/682629">Week 2 Part 2</a></td>
      <td>
        <ul>
          <li>ASCII Codes</li>
          <li>String in C</li>
          <li>Basic String Manipulations</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td><a href="./Week%203/Week_3_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99434/slides/682638">Week 3 Part 1</a></td>
      <td>
        <ul>
          <li>Preprocessor</li>
          <li>Compiler Flags</li>
          <li>C String Standard Library <code>string.h</code></li>
        </ul>
      </td>
    </tr>
    <tr>
      <td rowspan="2">Week 4</td>
      <td><a href="./Week%204/Week_4_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99434/slides/682646">Week 3 Part 2</a></td>
      <td>
        <ul>
          <li>More String Manipulations</li>
          <li>C Math Standard Library <code>math.h</code></li>
          <li>Bits</li>
          <li>Bitwise Operators</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td><a href="./Week%204/Week_4_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99435/slides/682658">Week 4 Part 1</a></td>
      <td>
        <ul>
          <li>Struct</li>
          <li>Union</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td rowspan="2">Week 5</td>
      <td><a href="./Week%205/Week_5_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99435/slides/682666">Week 4 Part 2</a></td>
      <td>
        <ul>
          <li>File Operations</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td><a href="./Week%205/Week_5_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99436/slides/682676">Week 5 Part 1</a></td>
      <td>
        <ul>
          <li>Stack and Heap</li>
          <li>Memory Management Functions</li>
          <li>Singly Linked List</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td rowspan="2">Week 6</td>
      <td><a href="./Week%206/Week_6_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99436/slides/682683">Week 5 Part 2</a></td>
      <td>
        <ul>
          <li>Doubly Linked List</li>
          <li>Circular Linked List</li>
          <li>Void Pointers</li>
          <li>More Exercises about Dynamic Memory Programming</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td><a href="./Week%206/Week_6_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99437/slides/682691">Week 6 Part 1</a></td>
      <td>
        <ul>
          <li>Device I/O and File Descriptors</li>
          <li>System Calls</li>
          <li>Non-blocking `STDIN`</li>
          <li>Function Pointers</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td rowspan="2">Week 7</td>
      <td><a href="./Week%207/Week_7_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99437/slides/716751">Week 6 Part 2</a></td>
      <td>
        <ul>
          <li>Function Pointer Exercises</li>
          <li>Signals</li>
          <li>Signal Safety</li>
          <li><code>signal()</code> and <code>sigaction()</code></li>
        </ul>
      </td>
    </tr>
    <tr>
      <td><a href="./Week%207/Week_7_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99446/slides/727224">Week 7 All</a></td>
      <td>
        <ul>
          <li>Compilation: C Compiling Process</li>
          <li>Linkage</li>
          <li><code>extern</code> and <code>static</code></li>
          <li><code>const</code> and <code>restrict</code></li>
          <li>Advanced C Programming Exercises</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td rowspan="2">Week 8</td>
      <td><del>Tutorial A</del></td>
      <td><del>N/A</del></td>
      <td><u><i>Assignment P1 Code Interview</u></i></td>
    </tr>
    <tr>
      <td><a href="./Week%208/Week_8_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99438/slides/682702">Week 8 Part 1</a></td>
      <td>
        <ul>
          <li>Process</li>
          <li>Race Conditions</li>
          <li><code>fork()</code>, <code>wait()</code>, and <code>exec*()</code></li>
        </ul>
      </td>
    </tr>
    <tr>
      <td rowspan="2">Week 9</td>
      <td><a href="./Week%209/Week_9_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99438/slides/682709">Week 8 Part 2</a></td>
      <td>
        <ul>
          <li>Safe <code>fork() + exec*()</code></li>
          <li>More About <code>sigaction()</code></li>
          <li>Process Exercises</li>
          <li>Advanced Signal Manipulation Exercises</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td><a href="./Week%209/Week_9_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99439/slides/682717">Week 9 Part 1</a></td>
      <td>
        <ul>
          <li>Inter-Process Communication (IPC)</li>
          <li>Message Passing: Pipes and <code>pipe()</code> in C</li>
          <li>File Descriptor Table</li>
          <li>IPC Exercises</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td rowspan="2">Week 10</td>
      <td><a href="./Week%2010/Week_10_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99439/slides/682723">Week 9 Part 2</a></td>
      <td>
        <ul>
          <li>Asynchronous I/O: Interrupt-Based vs <code>epoll</code>-Based</li>
          <li><code>select()</code>, <code>poll()</code>, and <code>epoll()</code></li>
          <li><code>socketpair()</code></li>
          <li>Shared Memory</li>
          <li>Semaphore</li>
          <li>Advanced IPC Exercises</li>
        </ul>
      </td>
    </tr>
    <tr>
      <td><a href="./Week%2010/Week_10_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="">Week 10 Part 1</a></td>
      <td><i>Tutorial notes have not yet been released.</i></td>
    </tr>
        <tr>
      <td rowspan="2">Week 11</td>
      <td><a href="./Week%2011/Week_11_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="">Week 10 Part 2</a></td>
      <td><i>Tutorial notes have not yet been released.</i></td>
    </tr>
    <tr>
      <td><a href="./Week%2011/Week_11_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="">Week 11 Part 1</a></td>
      <td><i>Tutorial notes have not yet been released.</i></td>
    </tr>
        <tr>
      <td rowspan="2">Week 12</td>
      <td><a href="./Week%2012/Week_12_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="">Week 11 Part 2</a></td>
      <td><i>Tutorial notes have not yet been released.</i></td>
    </tr>
    <tr>
      <td><a href="./Week%2012/Week_12_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="">Week 12 Part 1</a></td>
      <td><i>Tutorial notes have not yet been released.</i></td>
    </tr>
  </tbody>
</table>

---

### Copyright

Copyright &copy; 2026 by Hao Ren. All rights reserved.
