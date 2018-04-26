#include<stdio.h>
#include<stdlib.h>

#include"heap.h"
#include"linkedlist.h"
#include"atomiclist.h"

/*
 * tests the linked list unit.
 * Prints data structures to terminal for validation.
 */

struct ELEMENT
{
        union LINK Link;
        int Data;
};

struct LINKED_LIST queueList;
struct LINKED_LIST stackList;
struct ATOMIC_LIST atomicList;
struct HEAP heap;

void Add( int value )
{
        struct ELEMENT * queueNode = malloc( sizeof( struct ELEMENT ) );
        struct ELEMENT * stackNode = malloc( sizeof( struct ELEMENT ) );
        struct ELEMENT * atomicNode = malloc( sizeof( struct ELEMENT ) );
        struct ELEMENT * heapNode = malloc( sizeof( struct ELEMENT ) );

        queueNode->Data = value;
        stackNode->Data = value;
        atomicNode->Data = value;
        heapNode->Data = value;

        LinkedListPush( &stackNode->Link.LinkedListLink, &stackList );
        LinkedListEnqueue( &queueNode->Link.LinkedListLink, &queueList );
        AtomicListPush( &atomicNode->Link.AtomicListLink, &atomicList );
        HeapAdd(value, &heapNode->Link.WeightedLink, &heap );
}

struct ELEMENT * getElementFromLink(union LINK * link) {
        if(link == NULL) {
                return NULL;
        } else {
                return BASE_OBJECT( link, struct ELEMENT, Link );
        }
}

void Pop()
{
        struct ELEMENT * queueNode = getElementFromLink( (union LINK *) LinkedListPop( &queueList ));
        struct ELEMENT * stackNode = getElementFromLink( (union LINK *) LinkedListPop( &stackList ));
        struct ELEMENT * atomicNode = getElementFromLink( (union LINK *) AtomicListPop( &atomicList));
        struct ELEMENT * heapNode  = getElementFromLink( (union LINK *) HeapPop( &heap ));

        if( queueNode != NULL )
                printf("queue returned %d\n", queueNode->Data );
        else
                printf("queue returned null\n");

        if( stackNode != NULL )
                printf("stack returned %d\n", stackNode->Data );
        else
                printf("stack returned null\n");

        if( atomicNode != NULL )
                printf("atomic returned %d\n", atomicNode->Data );
        else
                printf("atomic returned null\n");

        if( heapNode != NULL )
                printf("heap returned  %d\n", heapNode->Data );
        else
                printf("heap returned null\n");
}

void DumpList(struct LINKED_LIST * list) {
  LINKED_LIST_FOR_EACH(cur, struct ELEMENT, Link) {
    printf("%d,", cur->Data);
  }
  printf("\n");
}

void DumpAtomic( struct ATOMIC_LIST * list )
{
        struct ELEMENT * cur = (struct ELEMENT *) list->Head.Tuple.Pointer;

        if( list->Head.Tuple.Pointer == NULL )
        {
                printf("null list\n");
                return;
        }

        do
        {
                printf("%d,", cur->Data);
                cur = (struct ELEMENT *) cur->Link.LinkedListLink.Next;
        }while( cur != NULL );
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
        LinkedListInit( & queueList );
        LinkedListInit( & stackList );
        AtomicListInit( & atomicList );
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
                        printf("Atomic:");
                        DumpAtomic( &atomicList );
                        DumpHeap( & heap );
                }
                else
                        Add(data);
        }while( data != 0 );
        return 0;
}
