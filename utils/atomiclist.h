#ifndef ATOMIC_LIST_H
#define ATOMIC_LIST_H

#include"link.h"
#include"types.h"
#include"utils.h"

struct ATOMIC_LIST
{
        //Head.Atomic is used for swap values.
        //Head.Tuple.Pointer is the Head Pointer
        //Head.Tuple.Atomic is the pop counter.
        union ATOMIC_UNION Head;
};

//Add to front of list.
void AtomicListPush( struct ATOMIC_LIST_LINK * node, struct ATOMIC_LIST * list );
//Pop next value off list.
struct ATOMIC_LIST_LINK * AtomicListPop( struct ATOMIC_LIST * list );

void AtomicListInit( struct ATOMIC_LIST * list );

#endif /* ATOMIC_LIST_H */
