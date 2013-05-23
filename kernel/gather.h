#ifndef GATHER_H
#define GATHER_H

#include"locking.h"
#include"utils/utils.h"
#include"utils/linkedlist.h"

struct GATHER
{
        volatile COUNT Needed;
        volatile COUNT Present;
        struct LINKED_LIST List;
};

void GatherInit( struct GATHER * gather, COUNT count );

void GatherSync( struct GATHER * gather, struct LOCKING_CONTEXT * context );

#endif
