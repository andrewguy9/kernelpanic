#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"
#include"kernel/range.h"
#include"kernel/gather.h"

//
//Main routine for threads.
//

#include<stdio.h>
struct GATHER RangeGuard;
struct RANGE_COROUTINE Range;

THREAD_MAIN ScratchMain;
void ScratchMain(void * arg ) {
  printf("Setting up routine\n");
  RangeRoutineInit(1, 1000000, 1, &Range);
  printf("Syncing bootstrap thread.\n");
  GatherSync( &RangeGuard, NULL );
  printf("killing bootstrap thread.\n");
}

THREAD_MAIN ThreadMain;
void ThreadMain(void * arg ) {
  struct RANGE_RESULT result;

  printf("Worker waiting for setup.\n");
  GatherSync( &RangeGuard, NULL );
  printf("Worker starting to run\n");
  result = RangeRoutineNext(&Range);
  while(result.State != RANGE_DONE) {
    printf("Cur: %lu\n", result.Last);
    result = RangeRoutineNext(&Range);
  }
  GeneralPanic();
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

struct THREAD Thread2;
char Thread2Stack[STACK_SIZE];

//
//Main
//

int main()
{
        //Initialize the kernel structures.
        KernelInit();

        SchedulerStartup();

        GatherInit( &RangeGuard, 2 );

        //Initialize Threads
        SchedulerCreateThread(
                        &Thread1,
                        2,
                        Thread1Stack,
                        STACK_SIZE,
                        ThreadMain,
                        NULL,
                        TRUE);

        SchedulerCreateThread(
                        &Thread2,
                        2,
                        Thread2Stack,
                        STACK_SIZE,
                        ScratchMain,
                        NULL,
                        TRUE);

        KernelStart();
        return 0;
}
