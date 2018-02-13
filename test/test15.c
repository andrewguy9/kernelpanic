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

  struct RANGE range;
  enum RANGE_STATUS status;
  INDEX cur;

  Range_Init(0, 1000000, 1, &range);

  for (status = RangeNext(&range, &cur);
      status!=RANGE_DONE;
      status = RangeNext(&range, &cur)) {
    printf("cur: %lu\n", cur);
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
