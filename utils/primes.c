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
#if 1
#define BUFFER_FOR_EACH(item, T, buff) \
  for (T * (item) = BufferNext((buff), (item)); \
      (item) != NULL; \
      (item) = BufferNext((buff), (item)))

  BUFFER_FOR_EACH(prime, int, data) {
#else
  for (int * prime = BufferNext(data, prime);
      prime != NULL;
      prime = BufferNext(data, prime)) {
#endif
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
