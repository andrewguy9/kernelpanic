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

union BLOCKING_CONTEXT * SchedulerGetBlockingContext( );

struct THREAD * SchedulerGetActiveThread();

void Schedule( ) ;

void SchedulerStartup( );

void 
SchedulerCreateThread( 
		struct THREAD * thread,
		unsigned char priority,
		char * stack,
		unsigned int stackSize,
		THREAD_MAIN main);

#endif
