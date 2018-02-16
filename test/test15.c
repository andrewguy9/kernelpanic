#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"
#include"kernel/coroutine.h"
#include"kernel/panic.h"

//
//Thread structures
//

#define STACK_SIZE HAL_MIN_STACK_SIZE

struct THREAD Thread1;
char Thread1Stack[STACK_SIZE];

char RangeStack[STACK_SIZE];

//
// Range Coroutine.
//

struct RANGE_PARAMS {
  INDEX Low;
  INDEX High;
  COUNT Step;
};
struct RANGE_RESULT {
  INDEX Last;
};

COROUTINE_FUNCTION RangeRoutine;
void RangeRoutine(
    void * params,
    void * result,
    struct COROUTINE_CONTEXT * yield) {
  struct RANGE_PARAMS * rparams = params;
  struct RANGE_RESULT * rresult = result;

  INDEX cur = rparams->Low;
  while (cur < rparams->High) {
    //Copy the last iteration to output.
    rresult->Last = cur;
    CoroutineYield(yield);
    // The co-routine was invoked via next.
    // Lets do more work.
    cur += rparams->Step;
  }
}

//
//Main routine for threads.
//

#include<stdio.h>
struct COROUTINE Range;

THREAD_MAIN ThreadMain;
void ThreadMain(void * arg ) {
  struct RANGE_PARAMS params = {0,1000000,1};
  struct RANGE_RESULT result;
  enum COROUTINE_STATUS status;

  printf("Setting up routine\n");
  CoroutineInit(
      RangeRoutine,
      &params,
      &result,
      RangeStack,
      STACK_SIZE,
      &Range);
  printf("routine ready\n");
  status = CoroutineNext(&Range);
  while(status != COROUTINE_DONE) {
    printf("Cur: %lu\n", result.Last);
    status = CoroutineNext(&Range);
  }
  printf("done\n");
  GeneralPanic( );
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
