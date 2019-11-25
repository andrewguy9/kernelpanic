#ifndef PRIMES_H
#define PRIMES_H

#include"utils/utils.h"

_Bool isPrime(int v);
_Bool isPrimeProduct(int v, int primes[], COUNT num_primes);
enum PRIMES_STATUS {PRIMES_OK, PRIMES_OVERFLOW};
enum PRIMES_STATUS findPrimes(int max, int primes[], COUNT primes_length);
#endif
