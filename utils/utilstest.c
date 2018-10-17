#include"utils.h"

#include<stdio.h>

void test_for_each_n() {
  int values[9] = {1,2,3,4,5,6,7,8,9};
  int sum = 0;
  FOR_EACH_N (i, values, 9) {
    printf("loop: %p = %i\n", i, *i);
    sum+=*i;
  }
  printf("sum %d\n", sum);
  ASSERT (sum == 45);
}

void test_for_each() {
  int values[9] = {1,2,3,4,5,6,7,8,9};
  int sum = 0;
  FOR_EACH (i, values) {
    printf("loop: %p = %i\n", i, *i);
    sum+=*i;
  }
  printf("sum %d\n", sum);
  ASSERT (sum == 45);
}
int main()
{
  test_for_each_n();
  test_for_each();
  return 0;
}
