// 2026 S1C COMP2017 Week 4 Tutorial A Demonstration
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Bits Operations.


#include <stdio.h>

void print_bits8(unsigned char x) {
    for (int i = 7; i >= 0; i--) {
        putchar((x & (1u << i)) ? '1' : '0');
    }
}

int main(void) {
    unsigned char a = 12;   // 00001100
    unsigned char b = 10;   // 00001010
    unsigned int flags = 0;

    printf("a      = ");
    print_bits8(a);
    printf(" (%u)\n", a);

    printf("b      = ");
    print_bits8(b);
    printf(" (%u)\n", b);

    printf("a & b  = ");
    print_bits8(a & b);
    printf(" (%u)\n", (unsigned)(a & b));

    printf("a | b  = ");
    print_bits8(a | b);
    printf(" (%u)\n", (unsigned)(a | b));

    printf("a | a  = ");
    print_bits8(a | a);
    printf(" (%u)\n", (unsigned)(a | a));

    printf("a ^ b  = ");
    print_bits8(a ^ b);
    printf(" (%u)\n", (unsigned)(a ^ b));

    printf("~a     = ");
    print_bits8((unsigned char)~a);
    printf(" (%u)\n", (unsigned)((unsigned char)~a));

    printf("a << 1 = ");
    print_bits8((unsigned char)(a << 1));
    printf(" (%u)\n", (unsigned)((unsigned char)(a << 1)));

    printf("a >> 1 = ");
    print_bits8((unsigned char)(a >> 1));
    printf(" (%u)\n", (unsigned)((unsigned char)(a >> 1)));

    flags |= 1u << 0;   // set bit 0
    flags |= 1u << 2;   // set bit 2

    printf("flags  = %u\n", flags);

    return 0;
}
