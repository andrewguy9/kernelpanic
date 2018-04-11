#ifndef PRIMES_H
#define PRIMES_H

#include"utils/utils.h"
#include"utils/buffer.h"

BOOL isPrime(int v);
BOOL isPrimeProduct(int v, DATA * primes);
enum PRIMES_STATUS {PRIMES_OK, PRIMES_OVERFLOW};
enum PRIMES_STATUS findPrimes(int max, int primes[], COUNT primes_length);
#endif
