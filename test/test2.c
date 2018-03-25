#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/panic.h"
#include"kernel/hal.h"
#include"kernel/watchdog.h"
#include"kernel/pipe.h"

/*
 * Tests the pipe unit, and by extension ringbuffer and semaphore units.
 * Will panic if reader encouners invalid read.
 */

#define MESSAGE_LENGTH 20
char Message[MESSAGE_LENGTH] = "Thread text message";

//Allocation for buffers.
#define RING_SIZE 1024
#define RING_TAG "test2_ring_buffer.map"

struct PIPE Pipe;
PIPE_READ PipeReader;
PIPE_WRITE PipeWriter;

//Allocation for workers.

#define STACK_SIZE HAL_MIN_STACK_SIZE

char ProducerStack1[STACK_SIZE];
char ProducerStack2[STACK_SIZE];

char ConsumerStack1[STACK_SIZE];
char ConsumerStack2[STACK_SIZE];
char ConsumerStack3[STACK_SIZE];

struct THREAD Producer1;
struct THREAD Producer2;

struct THREAD Consumer1;
struct THREAD Consumer2;
struct THREAD Consumer3;

#define QUANTUM 1
#define TIMEOUT (2*QUANTUM*5)

struct THREAD_CONTEXT
{
  INDEX WatchdogId;
};

struct THREAD_CONTEXT ProducerContext1 = {1};
struct THREAD_CONTEXT ProducerContext2 = {2};
struct THREAD_CONTEXT ConsumerContext1 = {3};
struct THREAD_CONTEXT ConsumerContext2 = {4};
struct THREAD_CONTEXT ConsumerContext3 = {5};

//Functions for test.
THREAD_MAIN ProducerMain;
void * ProducerMain(void * arg)
{
  struct THREAD_CONTEXT * context = (struct THREAD_CONTEXT *) arg;
  WatchdogAddFlag(context->WatchdogId);

  while (1) {
    PipeWriteStruct( Message, MESSAGE_LENGTH, &Pipe);
    WatchdogNotify(context->WatchdogId);
  }
  return NULL;
}

THREAD_MAIN ConsumerMain;
void * ConsumerMain(void * arg)
{
  struct THREAD_CONTEXT * context = (struct THREAD_CONTEXT *) arg;
  WatchdogAddFlag(context->WatchdogId);
  char buff[MESSAGE_LENGTH];

  COUNT index;
  while (1) {
    for (index = 0; index < MESSAGE_LENGTH; index++) {
      buff[index] = 0;
    }

    PipeReadStruct( buff, MESSAGE_LENGTH, &Pipe);

    for (index = 0; index < MESSAGE_LENGTH; index++) {
      if (Message[index] != buff[index]) {
        KernelPanic( );
      }
    }
    WatchdogNotify(context->WatchdogId);
  }
    return NULL;
}

//main
int main()
{
  void * RingBuff;
  KernelInit();

  SchedulerStartup();

  RingBuff = HalMap(RING_TAG, NULL, RING_SIZE);
  PipeInit( RingBuff, RING_SIZE, &Pipe, &PipeReader, &PipeWriter );

  SchedulerCreateThread(
      &Producer1,
      QUANTUM,
      ProducerStack1,
      STACK_SIZE,
      ProducerMain,
      & ProducerContext1,
      TRUE);
  SchedulerCreateThread(
      &Producer2,
      QUANTUM,
      ProducerStack2,
      STACK_SIZE,
      ProducerMain,
      & ProducerContext2,
      TRUE);
  SchedulerCreateThread(
      &Consumer1,
      QUANTUM,
      ConsumerStack1,
      STACK_SIZE,
      ConsumerMain,
      & ConsumerContext1,
      TRUE);
  SchedulerCreateThread(
      &Consumer2,
      QUANTUM,
      ConsumerStack2,
      STACK_SIZE,
      ConsumerMain,
      & ConsumerContext2,
      TRUE);

  SchedulerCreateThread(
      &Consumer3,
      QUANTUM,
      ConsumerStack3,
      STACK_SIZE,
      ConsumerMain,
      & ConsumerContext3,
      TRUE);

  WatchdogEnable( TIMEOUT );
  KernelStart();
  return 0;
}
