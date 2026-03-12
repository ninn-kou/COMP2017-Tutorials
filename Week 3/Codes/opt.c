// 2026 S1C COMP2017 Week 3 Tutorial B Extension Codes
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Optimization Flags, Try:
//     $ gcc -std=c23 -O0 opt.c -o opt_O0
//     $ gcc -std=c23 -O3 opt.c -o opt_O3
//     $ objdump -d opt_O0 | sed -n '/<main>:/,+8p'
//     $ objdump -d opt_O3 | sed -n '/<main>:/,+8p'


static int square(int x) {
    return x * x;
}

int main(void) {
    return square(5);
}
