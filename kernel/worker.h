#ifndef WORKER_H
#define WORKER_H

#include"thread.h"
#include"semaphore.h"

struct WORKER_ITEM;

enum WORKER_RETURN { WORKER_PENDED, WORKER_BLOCKED, WORKER_FINISHED };

typedef enum WORKER_RETURN (WORKER_FUNCTION) ( struct WORKER_ITEM * item );

struct WORKER_QUEUE
{
        struct THREAD Thread;
        struct LINKED_LIST List;
        struct SEMAPHORE Lock;
};

/*
 * Note: All of the elements in WORKER_ITEMs are only consistant from the
 * perspective of the work queue its assigned to.
 * The only exception to this is "Finished" which will always be updated atomically.
 */
struct WORKER_ITEM
{
        union LINK Link;
        struct WORKER_QUEUE * Queue;
        WORKER_FUNCTION * Foo;
        struct LOCKING_CONTEXT LockingContext;
        void * Context;
        _Bool Finished;//TODO IT WOULD BE NICE TO MOVE TO A DIFFERENT FINISHED MECHANISM, SAY READY LIKE THE HANDLERS?
};

void WorkerCreateWorker(
                struct WORKER_QUEUE * queue,
                char * stack,
                unsigned int stackSize);

void WorkerInitItem( struct WORKER_QUEUE * queue, WORKER_FUNCTION foo, void * context, struct WORKER_ITEM * item  );

_Bool WorkerItemIsFinished( struct WORKER_ITEM * item );

void * WorkerGetContext( struct WORKER_ITEM * item );

struct LOCKING_CONTEXT * WorkerGetLockingContext( struct WORKER_ITEM * item );
#endif
