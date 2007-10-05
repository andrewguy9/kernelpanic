#include<stdio.h>
#include<stdlib.h>

#include"heap.h"
#include"linkedlist.h"

struct ELEMENT
{
	union LINK Link;
	int Data;
};

struct LINKED_LIST queueList;
struct LINKED_LIST stackList;
struct HEAP heap;

void Add( int value )
{
	struct ELEMENT * queueNode = malloc( sizeof( struct ELEMENT ) );
	struct ELEMENT * stackNode = malloc( sizeof( struct ELEMENT ) );
	struct ELEMENT * heapNode = malloc( sizeof( struct ELEMENT ) );

	queueNode->Data = value;
	stackNode->Data = value;
	heapNode->Data = value;
	heapNode->Link.WeightedLink.Weight = value;

	LinkedListPush( (struct LINKED_LIST_LINK *) stackNode, &stackList );
	LinkedListEnqueue( (struct LINKED_LIST_LINK *) queueNode, &queueList );
	HeapAdd( (struct WEIGHTED_LINK *) heapNode, &heap );
}

void Pop()
{
	struct ELEMENT * queueNode = (struct ELEMENT *) LinkedListPop( &queueList );
	struct ELEMENT * stackNode = (struct ELEMENT *) LinkedListPop( &stackList );
	struct ELEMENT * heapNode = (struct ELEMENT *) HeapPop( &heap );
	
	if( queueNode != NULL )
		printf("queue returned %d\n", queueNode->Data );
	else
		printf("queue returned null\n");

	if( stackNode != NULL )
		printf("stack returned %d\n", stackNode->Data );
	else
		printf("stack returned null\n");

	if( heapNode != NULL )
		printf("heap returned  %d\n", heapNode->Data );
	else
		printf("heap returned null\n");
}

void DumpList( struct LINKED_LIST * list )
{
	struct ELEMENT * cur = (struct ELEMENT *) list->Head;
	struct ELEMENT * head = (struct ELEMENT *) list->Head;

	if( list->Head == NULL )
	{
		printf("null list\n");
		return;
	}

	do
	{
		printf("%d,", cur->Data);
		cur = (struct ELEMENT *) cur->Link.LinkedListLink.Next;
	}while( cur != head );
	printf("\n");
	return;
}

void TraverseHeap( struct WEIGHTED_LINK *link, int tab )
{
	int count;
	for( count = 0; count<tab; count++ )
		printf("   ");

	if( link != NULL )
	{
		printf("%ld\n", link->Weight);

		TraverseHeap( link->Left, tab+1 );
		TraverseHeap( link->Right, tab+1 );
	}
	else
	{
		printf("null node\n");
	}
}

void DumpHeap( struct HEAP * heap )
{
	printf("dumping heap\n");
	struct ELEMENT * head = (struct ELEMENT *) heap->Head;

	if( head == NULL )
		printf("empty heap\n");
	else
		TraverseHeap( (struct WEIGHTED_LINK *) head, 0 );
	printf("done\n\n");
}

int main()
{
	InitLinkedList( & queueList );
	InitLinkedList( & stackList );
	HeapInit( & heap );
	int data;
	do
	{
		printf("enter a data value:\n0 to exit\n1 to remove a node\n2 to print the data structures\nor a larger number to add a node.\n");
		scanf("%d", &data );
		if( data == 0 )
			return 0;
		else if( data == 1 )
			Pop();
		else if( data == 2 )
		{
			printf("queue:");
			DumpList( & queueList );
			printf("stack:");
			DumpList( & stackList );
			DumpHeap( & heap );
		}
		else
			Add(data);
	}while( data != 0 );
}
