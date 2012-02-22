#include"atomiclist.h"

void AtomicListPush( struct ATOMIC_LIST_LINK * node, struct ATOMIC_LIST * list )
{
        do {
                node->Next = list->Head;
        } while (node->Next != HalCompareAndSwapPointers(
                                &list->Head, // Swap this location
                                node->Next,  // If it equals this value
                                node));     // with this  value.
}

struct ATOMIC_LIST_LINK * AtomicListPop( struct ATOMIC_LIST * list )
{
        struct ATOMIC_LIST_LINK * link;
        struct ATOMIC_LIST_LINK * next;

        do {
                link = list->Head;

                if(!link) {
                        return NULL;
                }

                next = link->Next;
        } while (link != HalCompareAndSwapPointers(
                                &list->Head,
                                link,
                                next));

        return link;
}

void AtomicListInit( struct ATOMIC_LIST * list )
{
        list->Head = NULL;
}

