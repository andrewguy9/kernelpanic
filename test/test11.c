#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/pipe.h"
#include"kernel/panic.h"
#include"kernel/watchdog.h"
#include"utils/buffer.h"
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

DATA SetupData(char * buff, COUNT len, char start, int diff) {
  SPACE space = BufferSpace(buff, len);
  for (char val = start; !BufferFull(&space); val+=diff) {
    DATA valData = BufferFromObj(val);
    BufferCopy(&valData, &space);
  }
  ASSERT (BufferFull(&space));
  return BufferData(buff, &space);
}

char AssendingBuffer[RANDOM_VALUES_SIZE];
char DecendingBuffer[RANDOM_VALUES_SIZE];

DATA AssendingData;
DATA DecendingData;

THREAD_MAIN ProducerMain;
void * ProducerMain( void * arg )
{
  struct PRODUCER_CONTEXT * context = (struct PRODUCER_CONTEXT *) arg;
  WatchdogAddFlag(context->WatchdogId);

  INDEX timeIndex = 0;
  BOOL assending = TRUE;
  char * curBuffer;
  PIPE_WRITE writer = context->Writer;
  while (1) {
    COUNT length = RandomNumbers[timeIndex];

    if (assending) {
      curBuffer = AssendingBuffer;
    } else {
      curBuffer = DecendingBuffer;
    }

    DATA data = BufferSpace(curBuffer, length);
    PipeWriteStructBuff(&data, writer);

    timeIndex = (timeIndex + 1) % RANDOM_VALUES_SIZE;
    assending = !assending;
    WatchdogNotify(context->WatchdogId);
  }
  return NULL;
}

typedef BOOL CMP_FN(char *, char *);
CMP_FN IsAssending;
BOOL IsAssending(char * v1, char * v2) {
  return *v1 < *v2;
}

CMP_FN IsDecending;
BOOL IsDecending(char * v1, char * v2) {
  return *v1 > *v2;
}

THREAD_MAIN ConsumerMain;
void * ConsumerMain( void * arg )
{
  struct CONSUMER_CONTEXT * context = (struct CONSUMER_CONTEXT *) arg;
  PIPE_READ reader = context->Reader;
  INDEX timeIndex = 0;
  char myBuffer[RANDOM_VALUES_SIZE];
  BOOL assending = TRUE;

  WatchdogAddFlag(context->WatchdogId);

  while (1) {
    //Set Buffer up with values which will fail if a bug occurs.
    SPACE fillSpace = BufferSpace(myBuffer, RANDOM_VALUES_SIZE);
    char fillValue = assending ? 0 : RANDOM_VALUES_SIZE;
    while (!BufferFull(&fillSpace)) {
      DATA fill = BufferFromObj(fillValue);
      BufferCopy(&fill, &fillSpace);
    }

    COUNT length = RandomNumbers[timeIndex];

    //Perform read
    SPACE space = BufferSpace(myBuffer, length);
    PipeReadStructBuff(&space, reader);

    //validate direction of buffer.
    DATA data = BufferData(myBuffer, &space);
    CMP_FN * cmp = assending ? IsAssending : IsDecending;
    char * last = NULL;
    for (char * cur = BufferNext(data, cur);
        cur != NULL;
        cur = BufferNext(data, cur)) {
      if (last) {
        if (!cmp(last, cur)) {
            KernelPanic();
        }
      }
      last = cur;
    }

    //Setup next value.
    timeIndex = ( timeIndex + 1 ) % RANDOM_VALUES_SIZE;
    assending = !assending;
    WatchdogNotify(context->WatchdogId);
  }
  return NULL;
}

int main()
{
  AssendingData = SetupData(AssendingBuffer, RANDOM_VALUES_SIZE, 'a', 1);
  DecendingData = SetupData(DecendingBuffer, RANDOM_VALUES_SIZE, 'z', -1);

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
