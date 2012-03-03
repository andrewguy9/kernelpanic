#ifndef BLOCKING_CONTEXT_H
#define BLOCKING_CONTEXT_H

/*
 * A lock may have to store information about how the thread wanted to acquire the lock.
 * This is where that data should be stored. However it is important to keep this as small
 * as possible.
 */
enum RESOURCE_STATE
{
        RESOURCE_SHARED,
        RESOURCE_EXCLUSIVE
};

union BLOCKING_CONTEXT
{
        enum RESOURCE_STATE ResourceWaitState;
};

#endif
