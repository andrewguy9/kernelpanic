#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"
#include"kernel/coroutine.h"
#include"kernel/panic.h"

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

CO_ROUTINE_FUNCTION RangeRoutine;
void RangeRoutine(
    void * params,
    void * result,
    struct CO_ROUTINE_CONTEXT * yield) {
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
struct CO_ROUTINE Range;

THREAD_MAIN ThreadMain;
void ThreadMain(void * arg ) {
  struct RANGE_PARAMS params = {0,1000000,1};
  struct RANGE_RESULT result;
  enum CO_ROUTINE_STATUS status;

  printf("Setting up routine\n");
  CoroutineInit(
      RangeRoutine,
      &params,
      &result,
      &Range);
  printf("routine ready\n");
  status = CoroutineNext(&Range);
  while(status != CO_ROUTINE_DONE) {
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
//Thread structures
//

#define STACK_SIZE HAL_MIN_STACK_SIZE

struct THREAD Thread1;
char Thread1Stack[STACK_SIZE];

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
