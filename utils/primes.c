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

enum PRIMES_STATUS findPrimes(int max, SPACE * space) {
  SPACE orig = *space;
  //TODO WE WANT TO EXIT IF OVERFLOW, NOT IF FULL.
  for (int cur = 2; cur < max && !BufferFull(space); cur++) {
    //TODO It sucks that we have to keep building data up.
    DATA data = BufferData(orig.Buff, space);
    if (isPrimeProduct(cur, &data)) {
#if 0
      DATA prime = BufferFromObj(cur);
      BufferCopy(&prime, space);
#else
//TODO I DIDN'T DO A BOUNDS CHECK
#define BufferWrite(item, buffer) \
      do { \
        typeof(item) * src = &(item); \
        typeof(item) * dst = (typeof(cur) *) buffer->Buff; \
        *dst = *src; \
        buffer->Buff += sizeof(item); \
        buffer->Length -= sizeof(item); \
      } while(0)
      BufferWrite(cur, space);
#endif
    }
  }
  //TODO WE WANT TO ERROR ON OVERFLOW, NOT FULL.
  if (BufferFull(space)) {
    return PRIMES_OVERFLOW;
  } else {
    return PRIMES_OK;
  }
}
