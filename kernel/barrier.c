#include"barrier.h"

/*
 * Unit Description
 *
 * Barrier is a syncronization mechanism which allow
 * multiple threads to syncronize their actions.
 *
 * Barrier is like an inverted semaphore. It is initialized
 * to wait for n threads. Each thread walls BarrierSync and is 
 * blocked until all n threads are blocked. Then the lock awakens
 * all of them at the same time. This is useful if you need
 * multiple threads to move in lock step.
 */

/*
 * Initializes the barrier structure.
 * Count is the number of threads the barrier lock will block.
 * This should not be called on an active barrier lock.
 */
void BarrierInit( struct BARRIER * barrier, COUNT count )
{
        barrier->Needed = count;
        barrier->Present = 0;
        LinkedListInit( & barrier->List );
}

/*
 * Blocks the thread until all the threads have called BarrierSync.
 * When the final thread arrives, the lock will wake all threads in the barrier.
 */
void BarrierSync( struct BARRIER * barrier, struct LOCKING_CONTEXT * context )
{
        LockingStart();

        //Mark that new thread has arrived
        barrier->Present++;
        //see if everyone is here
        if( barrier->Needed == barrier->Present ) {
                //everyone is here
                barrier->Present = 0;
                //tell new guy that he can continue
                LockingAcquire( context );
                //Tell everyone waiting they can continue.
                while( ! LinkedListIsEmpty( & barrier->List ) ) {
                        struct LOCKING_CONTEXT * curContext;
                        curContext = container_of(
                                        LinkedListPop( &barrier->List ),
                                        struct LOCKING_CONTEXT,
                                        Link);
                        LockingAcquire( curContext );
                }
                //We are done, everyone is awake
                LockingEnd();
        } else {
                //We are not the last man,
                //so we need to block.
                union LINK * link;

                link = LockingBlock( NULL, context );

                //We need to store ourself away for later retreval.
                LinkedListEnqueue( &link->LinkedListLink, &barrier->List );

                //We may need to switch threads.
                LockingEnd();
        }
}
