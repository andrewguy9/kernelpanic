#include <stdio.h>

#define lambdaRef(l_ret_type, l_name, l_arguments) \
  l_ret_type (*l_name) l_arguments

#define lambda(l_ret_type, l_arguments, l_body)        \
  ({                                                   \
   l_ret_type l_anonymous_functions_name l_arguments   \
   l_body                                              \
   &l_anonymous_functions_name;                        \
   })

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


