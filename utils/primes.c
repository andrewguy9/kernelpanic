#include"utils/primes.h"
/*
 * Determines if a number is prime
 */

BOOL isPrime(int v)
{
  int i;
  for (i=0; i<v; i++) {
    if (v % i == 0) {
      return FALSE;
    }
  }
  return TRUE;
}

BOOL isPrimeProduct(int v, DATA * primes)
{
  DATA data = *primes;
  for (int * prime = BufferNext(data, prime);
      prime != NULL;
      prime = BufferNext(data, prime)) {
    if (v % *prime == 0) {
      return FALSE;
    }
  }
  return TRUE;
}

enum PRIMES_STATUS findPrimes(int max, int primes[], COUNT primes_length) {
  COUNT found_primes= 0;
  int cur;

  for (cur = 2; cur < max; cur++) {
    //TODO DOING POINTER PATH.
    DATA data = BufferSpace(primes, sizeof(int)*found_primes);
    if (isPrimeProduct(cur, &data)) {
      if (found_primes+1 > primes_length) {
        return PRIMES_OVERFLOW;
      } else {
        primes[found_primes++] = cur;
      }
    }
  }
  for (; found_primes<primes_length; found_primes++) {
    //TODO DOING POINTER MATH.
    primes[found_primes] = 0;
  }
  return PRIMES_OK;
}
