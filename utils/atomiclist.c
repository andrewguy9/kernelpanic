#include"atomiclist.h"

void AtomicListPush( struct ATOMIC_LIST_LINK * node, struct ATOMIC_LIST * list )
{
        do {
                // Get node pointing at the old head.
                node->Next = list->Head.Tuple.Pointer;

        } while ( node->Next != HalCompareAndSwap(
                                &list->Head.Tuple.Pointer, // Swap this location
                                list->Head.Tuple.Pointer,  // If it equals this value
                                node) );                     // with this  value.
}

struct ATOMIC_LIST_LINK * AtomicListPop( struct ATOMIC_LIST * list )
{
        return NULL;
}

void AtomicListInit( struct ATOMIC_LIST * list )
{
        list->Head.Tuple.Pointer = NULL;
        list->Head.Tuple.Generation = 0;
}

