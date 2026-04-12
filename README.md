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
      <td>—</td>
    </tr>
    <tr>
      <td><a href="./Week%206/Week_6_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99437/slides/682691">Week 6 Part 1</a></td>
      <td>—</td>
    </tr>
    <tr>
      <td rowspan="2">Week 7</td>
      <td><a href="./Week%207/Week_7_Tutorial_A.md">Tutorial A</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99437/slides/716751">Week 6 Part 2</a></td>
      <td>—</td>
    </tr>
    <tr>
      <td><a href="./Week%207/Week_7_Tutorial_B.md">Tutorial B</a></td>
      <td><a href="https://edstem.org/au/courses/31567/lessons/99446/slides/727224">Week 7 All</a></td>
      <td>—</td>
    </tr>
  </tbody>
</table>


---

### Copyright

Copyright &copy; 2026 by Hao Ren. All rights reserved.
