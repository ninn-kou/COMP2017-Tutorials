// 2026 S1C COMP2017 Week 3 Tutorial B Extension Codes
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// `-Wpointer-arith` Flag, Try:
//     $ gcc -std=gnu23 pointerarith.c -c
//     $ gcc -std=gnu23 -Wpointer-arith pointerarith.c -c
//     $ gcc -std=c23 -Wpedantic pointerarith.c -c


int main(void) {
    void *p = 0;
    p++;
    return 0;
}
