#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"
#include"kernel/range.h"

//
//Main routine for threads.
//

#include<stdio.h>
THREAD_MAIN ThreadMain;
void ThreadMain(void * arg ) {

  struct RANGE_RESULT result;
  result = RangeGlobal(TRUE, 0, 1000000, 1);
  while(result.State != RANGE_DONE) {
    printf("Cur: %lu\n", result.Last);
  result = RangeGlobal(FALSE, 0, 1000000, 1);
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
