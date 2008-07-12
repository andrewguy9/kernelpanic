#ifndef WORKER_H
#define WORKER_H

#include"thread.h"

struct WORKER_ITEM;

enum WORKER_RETURN { WORKER_QUEUED, WORKER_PENDED, WORKER_BLOCKED };

typedef enum WORKER_RETURN (* WORKER_FUNCTION) ( struct WORKER_ITEM * item );

struct WORKER_ITEM
{
	union LINK Link;
	WORKER_FUNCTION Foo;
	struct LOCKING_CONTEXT LockingContext;
	BOOL Finished;
	void * Context;
};

void WorkerStartup();

void WorkerCreateWorker(
		struct THREAD * thread,
		char * stack,
		unsigned int stackSize,
		char flag);

void WorkerInitItem( WORKER_FUNCTION foo, void * context, struct WORKER_ITEM * item  );

BOOL WorkerItemIsFinished( struct WORKER_ITEM * item );

#endif
