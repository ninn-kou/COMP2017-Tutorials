## COMP2017 2026 S1 Week 12 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

[TOC]

---

### B.1 Recursion & Recursion Overhead

Recursion is a way to solve a problem by reducing it into a smaller version of the same problem.

A recursive function usually has two parts:

```text
base case       stop condition
recursive step  call the same function on a smaller problem
```

The base case is important because recursion must eventually stop. Without a base case, or if the recursive step does not move toward the base case, the function keeps calling itself until the stack overflows.

And by calling the function itself inside the function, we could always move a step forward. In that case, I would say recursion is like the induction you have learning in mathematical proofs.

For example, we could use the recursive method to calculate the sum of an array, where the `if` condition is the base case.

```c
int sum(int *arr, int length) {
    if (length == 0) {
        return 0;
    }

    return arr[0] + sum(arr + 1, length - 1);
}
```

```text
sum([3, 4, 5])
= 3 + sum([4, 5])
= 3 + 4 + sum([5])
= 3 + 4 + 5
= 12
```

Recursion is often natural when the problem itself has a recursive structure.

Examples include:

```text
walking a linked list
walking a tree
binary search
divide-and-conquer algorithms
backtracking
filesystem traversal
```

For example, a linked list is naturally recursive:

```text
a list is either empty
or a node followed by another list
```

A binary tree is also recursive:

```text
a tree is either empty
or a node with a left subtree and right subtree
```

That is why recursive code can be very clear for trees:

```c
void print_tree(struct node *root) {
    if (root == NULL) {
        return;
    }

    print_tree(root->left);
    printf("%d\n", root->value);
    print_tree(root->right);
}
```

The function handles one node, then recursively handles the smaller subtrees.

#### B.1.1 Recursion Overhead

Recursion is elegant, but it has a cost. Every function call creates a new stack frame. A stack frame stores information needed for that call, such as:

```text
function parameters
local variables
return address
saved registers
temporary state
```

So with recursion, every recursive call adds another frame to the stack.

For example:

```c
sum(arr, 5)
sum(arr + 1, 4)
sum(arr + 2, 3)
sum(arr + 3, 2)
sum(arr + 4, 1)
```

There are five active calls before the recursion starts returning. This uses more stack memory than a simple loop.

#### B.1.2 Stack Overflow

The program stack has limited space.

If recursion goes too deep, too many stack frames are created. Eventually the stack can overflow, often causing a segmentation fault.

Example:

```c
void bad_recursion(void) {
    bad_recursion();
}
```

This function has no base case, so it keeps calling itself forever until the stack runs out.

**Even with a valid base case, recursion can still be too deep**.

```c
int sum(int *arr, int length) {
    if (length == 0) {
        return 0;
    }

    return arr[0] + sum(arr + 1, length - 1);
}
```

This works logically, but if `length` is extremely large, it may use too much stack.

> [!NOTE]
> For example, please try running the command `./fib 1000000` in our exercise, "Fibonacci Number - I". You should see `Segmentation fault (core dumped)` immediately, before it actually starts doing any calculation.

> Then, also try `./fib 500`. This command may look like it is stuck, but it is actually trying to calculate the result. A recursion depth of 500 () is not large enough to cause a stack overflow, but the computation is still too expensive to finish within the time limit.

#### B.1.3 Iterative version

The same array sum can be written with a loop:

```c
int sum_iterative(int *arr, int length) {
    int total = 0;

    for (int i = 0; i < length; i++) {
        total += arr[i];
    }

    return total;
}
```

This version uses one stack frame total, no matter how large the array is.

So for a simple linear task like summing an array, the iterative version is usually better in C:

```text
recursive version  clearer for demonstrating recursion
iterative version  less stack overhead and safer for large inputs
```

#### B.1.4 Important Distinction: Recursive Idea vs Recursive Implementation

Some algorithms are easiest to understand recursively, but that does not always mean recursion is the best implementation.

For example, this recursive definition is simple:

```text
sum(arr) = first element + sum(rest of array)
```

But the loop implementation is more efficient in C.

On the other hand, for tree traversal, recursion often stays natural and practical because each recursive call follows the shape of the tree.

So the question is not:

```text
Is recursion good or bad?
```

The better question is:

```text
Does recursion make this problem clearer, and is the recursion depth safe?
```

#### B.1.5 Tail recursion

A tail-recursive function is one where the recursive call is the last operation.

Example:

```c
int sum_tail(int *arr, int length, int acc) {
    if (length == 0) {
        return acc;
    }

    return sum_tail(arr + 1, length - 1, acc + arr[0]);
}
```

The recursive call is the final action.

Some languages or compilers can optimize tail recursion so it behaves like a loop. This is called tail-call optimization

---

### B.2 Exercise: Fibonacci Numbers - I



---

### B.3 

---

### B.4 

---

### B.5

