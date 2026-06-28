#include <stdio.h>

int main() {
    // Variable declarations with initializers
    int c = 10;
    int b = 15;
    int a;

    /* Compute a = b + c */
    a = b + c;

    int x;
    x = a * 2;

    int i;
    for (i = 0; i < 5; i++) {
        x = x + i;
    }

    if (x > 0) {
        a = x - 1;
    } else {
        a = 0;
    }

    printf("Result: %d\n", a);

    return 0;
}