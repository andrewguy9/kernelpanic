#include<stdio.h>
#include<stdlib.h>

#define lambda_inc \
  ^ int (int x) { return x + 1; }

#define forEach(T) \
  ^(void (^ fn)(T, int), T arr[], int n) { \
    for (int i=0; i<n; i++) { \
      fn(arr[i], i); \
    } \
  }

#define map(A,B) \
  ^(B (^fn)(A), A input[], B output[], int n) { \
    forEach(A)( ^(A val, int i) { \
      output[i] = fn(val); \
    }, input, n); \
}

#define filter(T) \
  ^int (int (^ fn)(T), T arr[], T out[], int n) { \
  int pos = 0; \
  for (int i=0; i<n; i++) { \
    if (fn(arr[i])) { \
      out[pos++] = arr[i]; \
    } \
  } \
  return pos; \
}

int main() {
  int input[4] = {1,2,3,4};
  int intermediate[4];
  int output[4];
  int result = lambda_inc(1);
  printf("%c\n", result);
  map(int,int)(^(int x) { return x+1; }, input, intermediate, 4);
  int count = filter(int)(^int (int x) { return x%2==0;}, intermediate, output, 4);
  forEach(int)(^(int val, int index) {
      printf("%d ", val);
  }, output, count);
  printf("\n");
}
