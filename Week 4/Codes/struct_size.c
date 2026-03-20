// 2026 S1C COMP2017 Week 4 Tutorial B Demonstration
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Demonstration about struct size padding.
// Copied from https://www.w3schools.com/c/c_structs_padding.php.


#include <stdio.h>

struct Example {
  char a;    // 1 byte
  int  b;    // 4 bytes
  char c;    // 1 byte
};

int main() {
  printf("Size of struct: %zu bytes\n", sizeof(struct Example));    // 12 bytes?
  return 0;
}
