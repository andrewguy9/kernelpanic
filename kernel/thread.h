#ifndef THREAD_H
#define THREAD_H

#include"utils/utils.h"
#include"locking.h"
#include"hal.h"
#include"context.h"

enum THREAD_STATE { THREAD_STATE_RUNNING, THREAD_STATE_BLOCKED, THREAD_STATE_DONE };

struct THREAD
{
        union LINK Link;
        COUNT Priority;
        enum THREAD_STATE State;//Running or blocked state of thread.
        struct LOCKING_CONTEXT LockingContext;//Info on why the thread is blocked.
        struct CONTEXT Stack;//Info on registers/stack for thread.
        THREAD_MAIN * Main;//The thread's main function ptr.
        void * Argument;//The thread's arguments.
};


#endif
