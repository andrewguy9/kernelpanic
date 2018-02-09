#include"utils/primes.h"
#include"kernel/hal.h"
#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/panic.h"
#include"kernel/gather.h"

#define PRIME_TAG1 "test13_primes1.map"
#define PRIME_TAG2 "test13_primes2.map"

#define STACK_SIZE HAL_MIN_STACK_SIZE

char PrimesStack1[STACK_SIZE];
struct THREAD PrimesThread1;

char PrimesStack2[STACK_SIZE];
struct THREAD PrimesThread2;

struct GATHER PrimesGather;

THREAD_MAIN PrimesMain;
void PrimesMain(void * context) {
  char * tag = context;
  INDEX max = 1000000;
  COUNT buffer_size = 100000;
  int * primes_buffer = HalMap(tag, NULL, sizeof(int) * buffer_size);
  enum PRIMES_STATUS status = findPrimes(max, primes_buffer, buffer_size);
  if (status == PRIMES_OK) {
    GatherSync( &PrimesGather, NULL);
    GeneralPanic(); //TODO SOME KIND OF SIGNAL THAT WE SUCCEEDED.
  } else {
    KernelPanic();
  }
}

int main(int argc, char ** argv)
{
  KernelInit();
  SchedulerStartup();
  GatherInit( &PrimesGather, 2);

  SchedulerCreateThread(
      &PrimesThread1,
      100,
      PrimesStack1,
      STACK_SIZE,
      PrimesMain,
      PRIME_TAG1,
      TRUE);

  SchedulerCreateThread(
      &PrimesThread2,
      100,
      PrimesStack2,
      STACK_SIZE,
      PrimesMain,
      PRIME_TAG1,
      TRUE);

  KernelStart();

  return 0;
}
