#ifndef THREAD_H
#define THREAD_H

#include"utils/utils.h"
#include"locking.h"
#include"resource.h"
#include"hal.h"
#include"context.h"

enum THREAD_STATE { THREAD_STATE_RUNNING, THREAD_STATE_BLOCKED, THREAD_STATE_DONE };

struct THREAD
{
        union LINK Link;
        unsigned char Priority;
        enum THREAD_STATE State;//Running or blocked state of thread.
        struct LOCKING_CONTEXT LockingContext;//Info on why the thread is blocked.
        struct RESOURCE ResultLock; //Held RESOURCE_EXCLUSIVE while thread is running. People waiting to get access to Result will acquire it as RESOURCE_SHARED.
        struct CONTEXT Stack;//Info on registers/stack for thread.
        THREAD_MAIN * Main;//The thread's main function ptr.
        void * Argument;//The thread's arguments.
        void * Result;
};


#endif
