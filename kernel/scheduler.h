#ifndef SCHEDULER_H
#define SCHEDULER_H

#include"../utils/link.h"
#include"../utils/utils.h"
#include"thread.h"

void SchedulerStartCritical( );

void SchedulerEndCritical( );

#ifdef DEBUG
BOOL SchedulerIsCritical();
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
		unsigned char priority,
		char * stack,
		COUNT stackSize,
		THREAD_MAIN main,
		void * Argument,
		INDEX debugFlag,
		BOOL start);

struct LOCKING_CONTEXT * SchedulerGetLockingContext();

#endif
