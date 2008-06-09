#ifndef SCHEDULER_H
#define SCHEDULER_H

#include"../utils/link.h"
#include"../utils/utils.h"
#include"hal.h"
#include"thread.h"

void SchedulerStartCritical( );

void SchedulerEndCritical( );

BOOL SchedulerIsCritical();

void SchedulerForceSwitch( );

void SchedulerResumeThread( struct THREAD * thread );

void SchedulerBlockThread( );

struct THREAD * SchedulerGetActiveThread();

void SchedulerStartup( );

void 
SchedulerCreateThread( 
		struct THREAD * thread,
		unsigned char priority,
		char * stack,
		unsigned int stackSize,
		THREAD_MAIN main,
		char flag,
		BOOL start);

struct LOCKING_CONTEXT * SchedulerGetLockingContext();

#endif
