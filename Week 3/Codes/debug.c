// 2026 S1C COMP2017 Week 3 Tutorial B Extension Codes
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Debugging Flags, Try:
//     $ gcc -std=c23 debug.c -o no_g
//     $ gcc -std=c23 -g debug.c -o with_g
//     $ readelf -S with_g | grep debug
//     $ readelf -S no_g | grep debug


int add(int a, int b) {
    int sum = a + b;
    return sum;
}

int main(void) {
    return add(2, 3);
}
