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
  BUFFER_FOR_EACH(prime, int, data) {
    if (v % *prime == 0) {
      return FALSE;
    }
  }
  return TRUE;
}

enum PRIMES_STATUS findPrimes(int max, SPACE * space) {
  SPACE orig = *space;
  for (int cur = 2; cur < max && !BufferFull(space); cur++) {
    DATA primes = BufferData(orig.Buff, space);
    if (isPrimeProduct(cur, &primes)) {
      BufferWrite(cur, *space);
    }
  }
  //TODO WE WANT TO ERROR ON OVERFLOW, NOT FULL.
  if (BufferFull(space)) {
    return PRIMES_OVERFLOW;
  } else {
    return PRIMES_OK;
  }
}
