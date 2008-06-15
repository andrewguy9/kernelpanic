#include"linkedlist.h"
#include"utils.h"

/*
 * Inserts node into a list that starts with list as its head.
 * If list is null, then creats a one element list around node.
 */
void LinkedListInsert( struct LINKED_LIST_LINK * node, struct LINKED_LIST_LINK * list )
{
	ASSERT( node != NULL );

	if( list == NULL )
	{//list is empty, so node is now only value.
		node->Next = node;
		node->Prev = node;
	}
	else
	{//insert into existing list
		node->Next = list;
		node->Prev = list->Prev;
		list->Prev->Next = node;
		list->Prev = node;
	}
}

/*
 * Removes link from a list.
 * Sets node up as a single element list
 */
void LinkedListRemove( struct LINKED_LIST_LINK * node )
{
	ASSERT( node != NULL );

	node->Prev->Next = node->Next;
	node->Next->Prev = node->Prev;
	node->Next = node;
	node->Prev = node;
}

//Add to front of list.
void LinkedListPush( struct LINKED_LIST_LINK * node, struct LINKED_LIST * list )
{
	ASSERT( list != NULL );

	LinkedListInsert( node, list->Head );
	list->Head = node;
}

//Pop next value off list.
struct LINKED_LIST_LINK * LinkedListPop( struct LINKED_LIST * list )
{
	struct LINKED_LIST_LINK * top;

	ASSERT( list != NULL );

	if( list->Head == NULL )
		return NULL;

	top = list->Head;
	if( list->Head == list->Head->Next )
	{//there is only one element.
		list->Head = NULL;
	}
	else
	{//there are multiple elements, move head down.
		list->Head = top->Next;
		LinkedListRemove( top );
	}
	return top;
}

//Add to the end of list.
void LinkedListEnqueue( struct LINKED_LIST_LINK * node, struct LINKED_LIST * list )
{
	ASSERT( list != NULL );

	LinkedListInsert( node, list->Head );
	list->Head = node->Next;
}

BOOL LinkedListIsEmpty( struct LINKED_LIST * list )
{
	ASSERT( list != NULL );

	if( list->Head == NULL )
		return TRUE;
	else 
		return FALSE;
}

void LinkedListInit( struct LINKED_LIST * list )
{
	list->Head = NULL;
}

struct LINKED_LIST_LINK * LinkedListPeek( struct LINKED_LIST * list )
{
	ASSERT( list != NULL );

	return list->Head;
}
