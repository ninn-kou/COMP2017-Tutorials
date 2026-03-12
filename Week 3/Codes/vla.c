// 2026 S1C COMP2017 Week 3 Tutorial B Extension Codes
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// VLA Warning Flag, Try:
//     $ gcc -std=gnu23 vla.c -c
//     $ gcc -std=gnu23 -Wvla vla.c -c
//     $ gcc -std=c90 -pedantic-errors vla.c -c


int f(int n) {
    int a[n];
    return a[0];
}
