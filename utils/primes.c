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

BOOL isPrimeProduct(int v, int primes[], COUNT num_primes)
{
  int i;
  for (i=2; i<num_primes; i++) {
    if (v % primes[i] == 0) {
      return FALSE;
    }
  }
  return TRUE;
}

enum PRIMES_STATUS findPrimes(int max, int primes[], COUNT primes_length) {
  COUNT found_primes= 0;
  int cur;

  for (cur = 2; cur < max; cur++) {
    if (isPrimeProduct(cur, primes, found_primes)) {
      if (found_primes+1 > primes_length) {
        return PRIMES_OVERFLOW;
      } else {
        primes[found_primes++] = cur;
      }
    }
  }
  for (; found_primes<primes_length; found_primes++) {
    primes[found_primes] = 0;
  }
  return PRIMES_OK;
}
