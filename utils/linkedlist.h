#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include"link.h"
#include"utils.h"
//Simple insert at the end of list.
void LinkedListInsert( struct LINKED_LIST_LINK * node, struct LINKED_LIST_LINK * list );
//Simple remove of a node from a list.
void LinkedListRemove( struct LINKED_LIST_LINK * node );

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

BOOL LinkedListIsEmpty( struct LINKED_LIST * list );

void LinkedListInit( struct LINKED_LIST * list );
#endif
