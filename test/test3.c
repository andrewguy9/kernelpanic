#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/resource.h"
#include"kernel/hal.h"
#include"kernel/panic.h"
#include"utils/buffer.h"

/*
 * Test of the resource unit and the non blocking code path.
 * Should panic on failure.
 */

//
//Tests Resources
//

#define BUFFER_SIZE 512
int Buffer[BUFFER_SIZE];
#define SEQUENCE_LENGTH 16
int Sequence[SEQUENCE_LENGTH]={1,3,5,7,11,13,17,19,23,27,29,31,37,39,41,43};

struct RESOURCE BufferLock;

COUNT TimesWritten;
COUNT TimesRead;

THREAD_MAIN Writer;
void * Writer( void * arg )
{
  struct LOCKING_CONTEXT block;
  INDEX sequenceIndex=0;

  LockingInit( &block, LockingBlockNonBlocking, LockingWakeNonBlocking );

  while (1) {
    ResourceLockExclusive( &BufferLock, &block);
    while( !LockingIsAcquired( &block ) ) { }
    //resource should be exlusive
    ASSERT( BufferLock.State == RESOURCE_EXCLUSIVE );

    sequenceIndex++;
    sequenceIndex%=SEQUENCE_LENGTH;

    INDEX index = 0;
    SPACE space = BufferSpace(Buffer, BUFFER_SIZE*sizeof(int));
    while (!BufferFull(&space)) {
      index++;
      INDEX value = index + Sequence[ sequenceIndex ];
      DATA data = BufferFromObj(value);
      BufferCopy(&data, &space);
    }

    ResourceUnlockExclusive( &BufferLock );

    SchedulerStartCritical();
    TimesWritten++;
    SchedulerForceSwitch();
  }
  return NULL;
}

THREAD_MAIN Reader;
void * Reader( void * arg )
{
  BOOL ready = FALSE;

  while (! ready) {
    SchedulerStartCritical();
    if (TimesWritten > 0) {
      ready = TRUE;
      SchedulerEndCritical();
    } else {
      SchedulerForceSwitch();
    }
  }

  while (1) {
    ResourceLockShared( &BufferLock, NULL );

    //the resource should be shared
    ASSERT( BufferLock.State == RESOURCE_SHARED );

    DATA data = BufferSpace(Buffer, BUFFER_SIZE * sizeof(int));
    char * last = NULL;
    for (char * cur = BufferNext(data, cur);
        cur != NULL;
        cur = BufferNext(data, cur)) {
      if ( last && *last+1 != *cur) {
        KernelPanic( );
      }
    }

    ResourceUnlockShared( &BufferLock );

    SchedulerStartCritical();
    TimesRead++;
    SchedulerForceSwitch();
  }
  return NULL;
}

struct THREAD Writer1;
struct THREAD Writer2;
struct THREAD Reader1;
struct THREAD Reader2;
struct THREAD Reader3;

#define STACK_SIZE HAL_MIN_STACK_SIZE

char Writer1Stack[STACK_SIZE];
char Writer2Stack[STACK_SIZE];
char Reader1Stack[STACK_SIZE];
char Reader2Stack[STACK_SIZE];
char Reader3Stack[STACK_SIZE];

int main()
{
  KernelInit();

  SchedulerStartup();

  TimesRead = 0;
  TimesWritten = 0;

  ResourceInit(& BufferLock, RESOURCE_SHARED);

  SchedulerCreateThread(
      & Reader1,
      5,
      Reader1Stack,
      STACK_SIZE,
      Reader,
      NULL,
      TRUE);
  SchedulerCreateThread(
      & Reader2,
      5,
      Reader2Stack,
      STACK_SIZE,
      Reader,
      NULL,
      TRUE);
  SchedulerCreateThread(
      & Reader3,
      5,
      Reader3Stack,
      STACK_SIZE,
      Reader,
      NULL,
      TRUE);
  SchedulerCreateThread(
      & Writer1,
      5,
      Writer1Stack,
      STACK_SIZE,
      Writer,
      NULL,
      TRUE);
  SchedulerCreateThread(
      & Writer2,
      5,
      Writer2Stack,
      STACK_SIZE,
      Writer,
      NULL,
      TRUE);

  KernelStart();
  return 0;
}
