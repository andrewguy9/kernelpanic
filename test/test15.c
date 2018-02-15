#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"
#include"kernel/range.h"
#include"kernel/panic.h"

//
//Main routine for threads.
//

#include<stdio.h>
struct RANGE_COROUTINE Range;

THREAD_MAIN ThreadMain;
void ThreadMain(void * arg ) {
  struct RANGE_RESULT result;

  printf("Setting up routine\n");
  RangeRoutineInit(1, 1000000, 1, &Range);
  printf("routine ready\n");
  result = RangeRoutineNext(&Range);
  while(result.State != RANGE_DONE) {
    printf("Cur: %lu\n", result.Last);
    result = RangeRoutineNext(&Range);
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
