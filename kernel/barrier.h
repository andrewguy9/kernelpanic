#ifndef BARRIER_H
#define BARRIER_H

#include"locking.h"
#include"utils/utils.h"
#include"utils/linkedlist.h"

struct BARRIER
{
        volatile COUNT Needed;
        volatile COUNT Present;
        struct LINKED_LIST List;
};

void BarrierInit( struct BARRIER * barrier, COUNT count );

void BarrierSync( struct BARRIER * barrier, struct LOCKING_CONTEXT * context );

#endif
