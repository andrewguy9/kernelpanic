#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"
#include"kernel/coroutine.h"
#include"kernel/panic.h"
#include"utils/buffer.h"
#include"kernel/serial.h"

#define CHOOSE_N 99
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
  INDEX i;
  for (i=0; i<CHOOSE_K; i++) {
    result->Nums[i] = 0;
  }
}

void combinationsPrint(struct COMBOS_RESULT * result) {
  int i;
  // IsrDisable(IRQ_LEVEL_MAX);
#define BUFF_SIZE 512
  char buff[BUFF_SIZE];
  struct BUFF_CURSOR str = BuffCursorInit(buff, BUFF_SIZE);
  struct BUFF_CURSOR remstr = str;

  for (i=CHOOSE_K-1; i>=0; i--) {
    remstr = BuffStrRender(&remstr, "%d\t", result->Nums[i]);
  }
  remstr = BuffStrRender(&remstr, "\n");
  ASSERT (! BuffFull(&remstr));

  SerialSafeStrWrite(&str, &remstr);

  // IsrEnable(IRQ_LEVEL_MAX);
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
void * ThreadMain(void * arg ) {
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
  return NULL;
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
  SerialStartup();

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
