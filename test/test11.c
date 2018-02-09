#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/pipe.h"
#include"kernel/panic.h"
#include"kernel/watchdog.h"
#include<stdio.h>

/*
 * Tests the pipe unit.
 */

#define BUFFER_LENGTH 10

char MessageBuffer1[BUFFER_LENGTH];
char MessageBuffer2[BUFFER_LENGTH];
char MessageBuffer3[BUFFER_LENGTH];

#define RANDOM_VALUES_SIZE 15
char RandomNumbers [RANDOM_VALUES_SIZE] =
{
	0xf, 0x1, 0x2,
	0x3, 0x0, 0x5,
	0x6, 0x7, 0x8,
   	0x9, 0xa, 0x0,
	0xc, 0xd, 0xe
};

struct PIPE Pipe1;
struct PIPE Pipe2;
struct PIPE Pipe3;

#define STACK_SIZE HAL_MIN_STACK_SIZE

char ProducerStack1[STACK_SIZE];
char ProducerStack2[STACK_SIZE];
char ProducerStack3[STACK_SIZE];

char ConsumerStack1[STACK_SIZE];
char ConsumerStack2[STACK_SIZE];
char ConsumerStack3[STACK_SIZE];

struct THREAD Producer1;
struct THREAD Producer2;
struct THREAD Producer3;

struct THREAD Consumer1;
struct THREAD Consumer2;
struct THREAD Consumer3;

struct PRODUCER_CONTEXT
{
        INDEX WatchdogId;
        PIPE_WRITE Writer;
};
struct CONSUMER_CONTEXT
{
        INDEX WatchdogId;
        PIPE_READ Reader;
};

struct PRODUCER_CONTEXT ProducerContext1;
struct PRODUCER_CONTEXT ProducerContext2;
struct PRODUCER_CONTEXT ProducerContext3;
struct CONSUMER_CONTEXT ConsumerContext1;
struct CONSUMER_CONTEXT ConsumerContext2;
struct CONSUMER_CONTEXT ConsumerContext3;

void SetupPipe(
    char * buff,
    struct PIPE * pipe,
    struct PRODUCER_CONTEXT * pc,
    struct CONSUMER_CONTEXT * cc,
    INDEX producer_watchdog,
    INDEX consumer_watchdog) {
  PIPE_READ reader;
  PIPE_WRITE writer;
  PipeInit( buff, BUFFER_LENGTH, pipe, &reader, &writer );
  pc->WatchdogId = producer_watchdog;
  pc->Writer = writer;
  cc->WatchdogId = consumer_watchdog;
  cc->Reader = reader;
}

#define QUANTUM 1
#define TIMEOUT (2*QUANTUM*6)

THREAD_MAIN ProducerMain;
void ProducerMain( void * arg )
{
  struct PRODUCER_CONTEXT * context = (struct PRODUCER_CONTEXT *) arg;
  PIPE_WRITE writer = context->Writer;
  INDEX timeIndex;
  INDEX bufferIndex;
  COUNT length;
  char AssendingBuffer[RANDOM_VALUES_SIZE];
  char DecendingBuffer[RANDOM_VALUES_SIZE];
  BOOL assending;

  char * curBuffer;

  WatchdogAddFlag(context->WatchdogId);

  for( bufferIndex = 0; bufferIndex < RANDOM_VALUES_SIZE; bufferIndex++ )
  {
    AssendingBuffer[ bufferIndex ] = bufferIndex;
    DecendingBuffer[ bufferIndex ] = RANDOM_VALUES_SIZE - bufferIndex;
  }

  timeIndex = 0;
  assending = TRUE;

  while(1)
  {
    length = RandomNumbers[timeIndex];

    if( assending )
      curBuffer = AssendingBuffer;
    else
      curBuffer = DecendingBuffer;

    //Perform write
    PipeWriteStruct(
        curBuffer,
        length,
        writer);

    //Setup next value.
    timeIndex = (timeIndex + 1) % RANDOM_VALUES_SIZE;
    assending = !assending;
    WatchdogNotify(context->WatchdogId);
  }
}

THREAD_MAIN ConsumerMain;
void ConsumerMain( void * arg )
{
  struct CONSUMER_CONTEXT * context = (struct CONSUMER_CONTEXT *) arg;
  PIPE_READ reader = context->Reader;
  INDEX timeIndex;
  COUNT bufferIndex;
  char myBuffer[RANDOM_VALUES_SIZE];
  BOOL assending;
  COUNT length;

  timeIndex = 0;
  assending = TRUE;

  WatchdogAddFlag(context->WatchdogId);

  while(1)
  {
    //Set Buffer up with values which will fail if a bug occurs.
    for( bufferIndex = 0; bufferIndex < RANDOM_VALUES_SIZE; bufferIndex++ )
    {
      if( assending )
        myBuffer[bufferIndex] = 0;
      else
        myBuffer[bufferIndex] = RANDOM_VALUES_SIZE;
    }

    length = RandomNumbers[timeIndex];

    //Perform read
    PipeReadStruct(
        myBuffer,
        length,
        reader);

    //validate direction of buffer.
    for( bufferIndex = 0; bufferIndex+1 < length; bufferIndex++)
    {
      if( assending )
      {
        if( myBuffer[bufferIndex] >= myBuffer[bufferIndex+1] )
          KernelPanic();
      }
      else
      {
        if( myBuffer[bufferIndex] <= myBuffer[bufferIndex+1] )
          KernelPanic();
      }
    }

    //Setup next value.
    timeIndex = ( timeIndex + 1 ) % RANDOM_VALUES_SIZE;
    assending = !assending;
    WatchdogNotify(context->WatchdogId);
  }
}

int main()
{
  KernelInit();
  SchedulerStartup();
  SetupPipe(MessageBuffer1, &Pipe1, &ProducerContext1, &ConsumerContext1, 1, 2);
  SetupPipe(MessageBuffer2, &Pipe2, &ProducerContext2, &ConsumerContext2, 3, 4);
  SetupPipe(MessageBuffer3, &Pipe3, &ProducerContext3, &ConsumerContext3, 5, 6);

  SchedulerCreateThread(
      &Producer1,
      QUANTUM,
      ProducerStack1,
      STACK_SIZE,
      ProducerMain,
      & ProducerContext1,
      TRUE );
  SchedulerCreateThread(
      &Producer2,
      QUANTUM,
      ProducerStack2,
      STACK_SIZE,
      ProducerMain,
      & ProducerContext2,
      TRUE );
  SchedulerCreateThread(
      &Producer3,
      QUANTUM,
      ProducerStack3,
      STACK_SIZE,
      ProducerMain,
      & ProducerContext3,
      TRUE );
  SchedulerCreateThread(
      &Consumer1,
      QUANTUM,
      ConsumerStack1,
      STACK_SIZE,
      ConsumerMain,
      & ConsumerContext1,
      TRUE );
  SchedulerCreateThread(
      &Consumer2,
      QUANTUM,
      ConsumerStack2,
      STACK_SIZE,
      ConsumerMain,
      & ConsumerContext2,
      TRUE );
  SchedulerCreateThread(
      &Consumer3,
      QUANTUM,
      ConsumerStack3,
      STACK_SIZE,
      ConsumerMain,
      & ConsumerContext3,
      TRUE );

  WatchdogEnable( TIMEOUT );
  KernelStart();
  return 0;
}
