#include<stdio.h>
#include<stdlib.h>

int main() {
  int input[4] = {1,2,3,4};
  int intermediate[4];
  int output[4];
  
  void printr(int x) {
    printf("%d", x);
  }
  ({void _(int arr[], int n) {
    for (int i=0; i<n; i++) {
      printf("%d", arr[i]);
    }
  } (void (*)(int [], int))_;})(output, 4);
  printf("\n");
}
