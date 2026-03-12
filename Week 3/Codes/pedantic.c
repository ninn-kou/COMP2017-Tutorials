// 2026 S1C COMP2017 Week 3 Tutorial B Extension Codes
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// GCC Language-Standard and Conformance Flags, Try:
//     $ gcc pedantic.c -c
//     $ gcc -std=c23 -Wpedantic pedantic.c -c
//     $ gcc -std=c23 -pedantic-errors pedantic.c -c


int main(void) {
    int x = ({ int y = 3; y + 1; });
    return x;
}
