#ifndef SCHEDULER_H
#define SCHEDULER_H

#include"critinterrupt.h"
#include"utils/link.h"
#include"utils/utils.h"
#include"thread.h"

#define SchedulerStartCritical() (CritInterruptDisable())

#define SchedulerEndCritical() (CritInterruptEnable())

#ifdef DEBUG
#define SchedulerIsCritical() (CritInterruptIsAtomic())
#endif

void SchedulerForceSwitch( );

_Bool SchedulerIsThreadDead( struct THREAD * thread );

_Bool SchedulerIsThreadBlocked( struct THREAD * thread );

void SchedulerResumeThread( struct THREAD * thread );

void SchedulerBlockThread( );

struct THREAD * SchedulerGetActiveThread();

void SchedulerStartup( );

void SchedulerShutdown( );
_Bool SchedulerIsShuttingDown( );

void SchedulerCreateThread(
                struct THREAD * thread,
                unsigned char priority,
                char * stack,
                COUNT stackSize,
                THREAD_MAIN main,
                void * Argument,
                void * Local,
                _Bool start);

struct LOCKING_CONTEXT * SchedulerGetLockingContext();

void * SchedulerJoinThread(struct THREAD * thread);
void SchedulerReleaseThread(struct THREAD * thread);

void * ThreadLocalGet();
#define THREAD_LOCAL_GET(type) ( (type) ThreadLocalGet() )
void ThreadLocalSet(void * val);

#endif
