#include"utils/primes.h"

#include<stdio.h>
#include<stdlib.h>


int main(int argc, char ** argv)
{
  INDEX max = atoi(argv[1]);
  COUNT buffer_size = atoi(argv[2]);
  int * primes_buffer = malloc(sizeof(int) * buffer_size);
  SPACE space = BufferSpace(primes_buffer, buffer_size);

  enum PRIMES_STATUS status = findPrimes(max, &space);
  if (status == PRIMES_OK) {
    DATA primes = BufferData(primes_buffer, &space);
    BUFFER_FOR_EACH(prime, int, primes) {
      printf("%d\n", *prime);
    }
  } else {
    printf("Overflowed primes buffer\n");
  }
  return 0;
}
