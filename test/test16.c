#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"
#include"kernel/coroutine.h"
#include"kernel/panic.h"

#define CHOOSE_N 100
#define CHOOSE_K 10

//
//Thread structures
//

#define STACK_SIZE HAL_MIN_STACK_SIZE

struct THREAD Thread1;
char Thread1Stack[STACK_SIZE];

char CombosStack[STACK_SIZE];

//
// Combos Coroutine.
//

struct COMBOS_PARAMS {
  COUNT N;
  COUNT K;
};

struct COMBOS_RESULT {
  int Nums[CHOOSE_K];
};

void combinationsResultInit(struct COMBOS_RESULT * result) {
  for (INDEX i=0; i<CHOOSE_K; i++) {
    result->Nums[i] = 0;
  }
}

#include<stdio.h>
void combinationsPrint(struct COMBOS_RESULT * result) {
  int i;
  for (i=CHOOSE_K-1; i>=0; i--) {
    printf("%d\t", result->Nums[i]);
  }
  printf("\n");
}

void CombosRoutineInner(
    int,
    int,
    int,
    struct COMBOS_RESULT *,
    struct COROUTINE_CONTEXT *);
void CombosRoutineInner(
    int n,
    int k,
    int start,
    struct COMBOS_RESULT * result,
    struct COROUTINE_CONTEXT * yield) {
  INDEX i;
  for (i = start; i<n; i++) {
    result->Nums[k] = i+1;
    if (k == 0) {
      CoroutineYield(yield);
    } else {
      CombosRoutineInner(
          n,
          k-1,
          i+1,
          result,
          yield);
    }
  }
}

COROUTINE_FUNCTION CombosRoutine;
void CombosRoutine(
    void * params,
    void * result,
    struct COROUTINE_CONTEXT * yield) {
  struct COMBOS_PARAMS * cparams = params;

  int n = cparams->N;
  int k = cparams->K - 1;
  int start = 0;

  CombosRoutineInner(n, k, start, result, yield);
}

//
//Main routine for threads.
//

struct COROUTINE Combos;

THREAD_MAIN ThreadMain;
void ThreadMain(void * arg ) {
  struct COMBOS_PARAMS params = {CHOOSE_N, CHOOSE_K};
  struct COMBOS_RESULT result;
  enum COROUTINE_STATUS status;

  combinationsResultInit(& result);
  CoroutineInit(
      CombosRoutine,
      &params,
      &result,
      CombosStack,
      STACK_SIZE,
      &Combos);
  for(status = CoroutineNext(&Combos);
      status != COROUTINE_DONE;
      status = CoroutineNext(&Combos)) {
    combinationsPrint(& result);
  }
  SchedulerShutdown();
}

//
//Vars
//

//
//Main
//

int main()
{
  //Initialize the kernel structures.
  KernelInit();

  SchedulerStartup();

  //Initialize Threads
  SchedulerCreateThread(
      &Thread1,
      2,
      Thread1Stack,
      STACK_SIZE,
      ThreadMain,
      NULL,
      TRUE);

  KernelStart();
  return 0;
}
