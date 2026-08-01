#include <stdio.h>

int main() {

    int p, q, r, total;
    scanf("%d%d%d", &p, &q, &r/*hello*/);

    // total combines a product and a sum
    total = p * q + r * 10;

    printf("Total = %d\n", total);
    return 0;
}