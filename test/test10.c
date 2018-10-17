#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"
#include"kernel/serial.h"

/*
 * Reads from STDIN and prints the results to STDOUT.
 */

//
//Main routine for threads.
//

#define BUF_SIZE 32

THREAD_MAIN TestThreadMain;
void * TestThreadMain( void * arg )
{
  char buf[BUF_SIZE];

  while (1) {
    SPACE space = BufferSpace(buf, BUF_SIZE);
    SerialReadBuffer(&space);
    DATA data = BufferData(buf, &space);
    SerialWriteBuffer(&data);
  }
  return NULL;
}


//
//Thread structures
//

#define STACK_SIZE HAL_MIN_STACK_SIZE

struct THREAD TestThread;
char TestThreadStack[STACK_SIZE];

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
      &TestThread,
      2,
      TestThreadStack,
      STACK_SIZE,
      TestThreadMain,
      NULL,
      TRUE);

  KernelStart();
  return 0;
}
