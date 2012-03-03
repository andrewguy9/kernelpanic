#ifndef SIGNAL_H
#define SIGNAL_H

#include"../utils/utils.h"
#include"../utils/linkedlist.h"
#include"locking.h"

struct SIGNAL
{
        BOOL State;
        struct LINKED_LIST WaitingThreads;
};

void SignalInit( struct SIGNAL * signal, BOOL state );
void SignalSet( struct SIGNAL * signal );
void SignalUnset( struct SIGNAL * signal );
void SignalWaitForSignal( struct SIGNAL * signal, struct LOCKING_CONTEXT * context );

#endif
