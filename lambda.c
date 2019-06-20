#include <stdio.h>
#include"blocks.h"

void test1() {
    lambdaRef(int, fn, (int)) = lambda(int, (int x), {return x+1;});
    printf("out1 %d\n", fn(5));
}

void test2() {
    int x = lambda(int, (int x), {return x+1;})(5);
    printf("out2 %d\n", x);
}


int main()
{
    test1();
    test2();
    return 0;
}


