#ifndef SCHEDULER_H
#define SCHEDULER_H

#include"../utils/link.h"
#include"../utils/utils.h"
#include"timer.h"

typedef void (*THREAD_MAIN) ();

enum THREAD_STATE { THREAD_STATE_RUNNING, THREAD_STATE_BLOCKED };

union BLOCKING_CONTEXT
{
	COUNT SemaphoreCountNeeded;
	struct TIMER SleepTimer;
};

struct THREAD 
{
	union LINK Link;
	unsigned char Priority;
	enum THREAD_STATE State;
	union BLOCKING_CONTEXT BlockingContext;
	char * Stack;
};

void SchedulerStartCritical( );
void SchedulerEndCritical( );
void SchedulerForceSwitch( );
void SchedulerResumeThread( struct THREAD * thread );
void SchedulerBlockThread( );
void Schedule( ) ;
void SchedulerInit( );
struct THREAD * SchedulerGetActiveThread( );
void SchedulerCreateThread( 
		struct THREAD * thread,
		unsigned char priority,
		char * stack,
		unsigned int stackSize,
		THREAD_MAIN main);

#endif
