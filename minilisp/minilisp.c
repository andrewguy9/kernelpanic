#include"kernel/minilisp.h"

#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include "kernel/hal.h"
#include "kernel/serial.h"
#include "kernel/sleep.h"

#define STACK_SIZE HAL_MIN_STACK_SIZE

struct THREAD LispThread;
char LispThreadStack[STACK_SIZE];

// The size of the heap in bytes
#define MEMORY_SIZE 65536
char heap1[MEMORY_SIZE];
char heap2[MEMORY_SIZE];

int main() {
  struct ALLOC_BLOCK block;
  KernelInit();
  SerialStartup();
  SchedulerStartup();
  lisp_init(
      &block,
      heap1,
      heap2,
      MEMORY_SIZE,
      false,
      false);
  SchedulerCreateThread(
      &LispThread,
      255,
      LispThreadStack,
      STACK_SIZE,
      lisp_repl_main,
      NULL,
      &block,
      true);
  KernelStart();
  return 0;
}

