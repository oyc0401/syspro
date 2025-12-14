#include <stdio.h>
#include <stdlib.h>
#include "big.h"

static void run(const char* a, const char* b) {
    char* r1 = big_add(a, b);
    char* r2 = big_sub(a, b);

    printf("%s + %s = %s\n", a, b, r1);
    printf("%s - %s = %s\n", a, b, r2);

    free(r1);
    free(r2);
}

int main(void) {
    run("999999999999999999999999", "1");
    run("-4324324132541242136783218", "23214132421748912392173982179");
    return 0;
}
