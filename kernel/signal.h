#ifndef SIGNAL_H
#define SIGNAL_H

#include"../utils/utils.h"
#include"../utils/linkedlist.h"
#include"locking.h"

struct SIGNAL
{
	BOOLEAN State;
	struct LINKED_LIST WaitingThreads;
};

void SignalInit( struct SIGNAL * signal, BOOLEAN state );
void SignalSet( struct SIGNAL * signal );
void SignalUnset( struct SIGNAL * signal );
void SignalWaitOnSignal( struct * signal, struct LOCKING_CONTEXT context );

#endif
