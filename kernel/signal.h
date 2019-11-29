#ifndef SIGNAL_H
#define SIGNAL_H

#include"utils/utils.h"
#include"utils/linkedlist.h"
#include"locking.h"

struct SIGNAL
{
        _Bool State;
        struct LINKED_LIST WaitingThreads;
};

void SignalInit( struct SIGNAL * signal, _Bool state );
void SignalSet( struct SIGNAL * signal );
void SignalUnset( struct SIGNAL * signal );
void SignalWaitForSignal( struct SIGNAL * signal, struct LOCKING_CONTEXT * context );

#endif
