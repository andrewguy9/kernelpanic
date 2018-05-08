#include"utils/primes.h"

#include<stdio.h>
#include<stdlib.h>


int main(int argc, char ** argv)
{
  INDEX max = atoi(argv[1]);
  COUNT buffer_size = atoi(argv[2]);
  int * primes_buffer = malloc(sizeof(int) * buffer_size);
  SPACE space = BufferSpace(primes_buffer, buffer_size);

  INDEX i;
  enum PRIMES_STATUS status = findPrimes(max, &space);
  if (status == PRIMES_OK) {
    //TODO should be buffer loop.
    for (i=0; i<buffer_size; i++) {
      if (primes_buffer[i] == 0) {
        break;
      }
      printf("%d\n", primes_buffer[i]);
    }
  } else {
    printf("Overflowed primes buffer\n");
  }
  return 0;
}
