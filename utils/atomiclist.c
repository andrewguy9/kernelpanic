#include"atomiclist.h"

void AtomicListPush( struct ATOMIC_LIST_LINK * node, struct ATOMIC_LIST * list )
{
        do {
                // Get node pointing at the old head.
                node->Next = list->Head.Tuple.Pointer;

        } while ( (void *) node->Next != (void *) CompareAndSwapPtrs(
                                &list->Head.Tuple.Pointer, // Swap this location
                                list->Head.Tuple.Pointer,  // If it equals this value
                                node) );                     // with this  value.
}

struct ATOMIC_LIST_LINK * AtomicListPop( struct ATOMIC_LIST * list )
{
        union ATOMIC_UNION oldHead;
        union ATOMIC_UNION newHead;

        struct ATOMIC_LIST_LINK * node;
        struct ATOMIC_LIST_LINK * next;
        COUNT generation;

        do {
                // Get access to logical head.
                oldHead.Atomic = list->Head.Atomic;
                node = oldHead.Tuple.Pointer;
                generation = oldHead.Tuple.Generation;

                if(node == NULL) {
                        return NULL;
                }

                // Determine new head values.
                next = node->Next;
                generation++;

                // Attempt a writeback.
                newHead.Tuple.Pointer = next;
                newHead.Tuple.Generation = generation;
        } while (oldHead.Atomic != DoubleCompareAndSwap(
                                &list->Head.Atomic,
                                oldHead.Atomic,
                                newHead.Atomic) );

        return node;
}

void AtomicListInit( struct ATOMIC_LIST * list )
{
        list->Head.Tuple.Pointer = NULL;
        list->Head.Tuple.Generation = 0;
}

