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

BOOL SchedulerIsThreadDead( struct THREAD * thread );

BOOL SchedulerIsThreadBlocked( struct THREAD * thread );

void SchedulerResumeThread( struct THREAD * thread );

void SchedulerBlockThread( );

struct THREAD * SchedulerGetActiveThread();

void SchedulerStartup( );

void
SchedulerCreateThread(
                struct THREAD * thread,
                COUNT priority,
                char * stack,
                COUNT stackSize,
                THREAD_MAIN main,
                void * Argument,
                BOOL start);

struct LOCKING_CONTEXT * SchedulerGetLockingContext();

#endif
