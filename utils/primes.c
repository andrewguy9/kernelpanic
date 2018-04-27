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
    DATA data = BufferData(orig.Buff, buffer);
    if (isPrimeProduct(cur, &data)) {
      if (BufferFull(buffer)) {
        return PRIMES_OVERFLOW;
      } else {
        DATA prime = BufferFromObj(cur);
        //TODO THIS IS NOT SAFE.
        BufferCopy(&prime, buffer);
      }
    }
  }
  return PRIMES_OK;
}
