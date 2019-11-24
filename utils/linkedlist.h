#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include"link.h"
#include"utils.h"

struct LINKED_LIST
{
        struct LINKED_LIST_LINK * Head;
};

//Add to front of list.
void LinkedListPush( struct LINKED_LIST_LINK * node, struct LINKED_LIST * list );
//Pop next value off list.
struct LINKED_LIST_LINK * LinkedListPop( struct LINKED_LIST * list );
//Add to the end of list.
void LinkedListEnqueue( struct LINKED_LIST_LINK * node, struct LINKED_LIST * list );

struct LINKED_LIST_LINK * LinkedListPeek( struct LINKED_LIST * list );

_Bool LinkedListIsEmpty( struct LINKED_LIST * list );

void LinkedListInit( struct LINKED_LIST * list );

#endif
