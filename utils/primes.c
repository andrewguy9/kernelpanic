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

enum PRIMES_STATUS findPrimes(int max, SPACE * buffer) {
  SPACE orig = *buffer;
  for (int cur = 2; cur < max; cur++) {
    DATA primes = BufferData(orig.Buff, buffer);
    if (isPrimeProduct(cur, &primes)) {
      if (BufferFull(buffer)) {
        return PRIMES_OVERFLOW;
      } else {
        BufferWrite(cur, *buffer);
      }
    }
  }
  return PRIMES_OK;
}
