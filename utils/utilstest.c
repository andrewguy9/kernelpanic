#include"utils.h"

#include<stdio.h>
void test_foreach() {
  int values[9] = {1,2,3,4,5,6,7,8,9};
  int sum = 0;
  FOR_EACH (int * i, values) {
    printf("loop: %p = %i\n", i, *i);
    sum+=*i;
  }
  printf("sum %d\n", sum);
  ASSERT (sum == 45);
}

int main()
{
  test_foreach();
  return 0;
}
