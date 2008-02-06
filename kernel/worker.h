#ifndef WORKER_H
#define WORKER_H

#include"thread.h"
#include"handler.h"

void WorkerStartup();

void WorkerCreateWorker(
		struct THREAD * thread,
		char * stack,
		unsigned int stackSize,
		char flag);

void WorkerAddItem( HANDLER_FUNCTION foo, void * arg, struct HANDLER_OBJECT * obj );

#endif
