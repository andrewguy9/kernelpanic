#include<stdio.h>
#include<stdlib.h>

int main() {
  int input[4] = {1,2,3,4};
  int intermediate[4];
  int output[4];
  
  void printr(int x) {
    printf("%d", x);
  }
  for (int i=0; i<4; i++) {
     printr(output[i]);
  }
  printf("\n");
}
