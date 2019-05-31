#include<stdio.h>
#include<stdlib.h>

#define lambda_void_int_int(body) \
  ({void _(int x, int idx) body (void (*)(int, int))_;})

#define forEach(T) \
  ({void _(void (*fn)(T, int), T arr[], int n) { \
    for (int i=0; i<n; i++) { \
      fn(arr[i], i); \
    } \
  } (void (*)(void (*)(T, int), T [], int))_;})

int main() {
  int input[4] = {1,2,3,4};
  int intermediate[4];
  int output[4];

  lambda_void_int_int( {printf("%d %d\n", x, idx);} )(5,10);

  forEach(int)(
  ({void _(int x, int idx) { printf("%d", x); } (void (*)(int, int))_;}),
    output,
    4);
  printf("\n");
}
