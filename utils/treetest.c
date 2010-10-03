#include<stdio.h>
#include<stdlib.h>

#include"avl.h"
#include"linkedlist.h"

/*
 * tests the tree units
 * Prints data structures to terminal for validation.
 */

struct ELEMENT
{
	union LINK Link;
	int Data;
};

struct AVL_TREE avl;

void Add( int value )
{
	struct ELEMENT * element = malloc( sizeof( struct ELEMENT ) );

	element->Data = value;

	AvlAdd(value, &element->Link.WeightedLink, &avl );
}

void Remove(int value)
{
	union LINK * avlLink = (union LINK *) AvlFind(value, &avl);

	if( avlLink != NULL )
	{
		struct ELEMENT * element = BASE_OBJECT( 
				avlLink,
				struct ELEMENT, 
				Link);

		ASSERT( element->Data == value );

		printf("avl returned  %d\n", element->Data );
		AvlDelete( &element->Link.WeightedLink, &avl );
	}
	else
	{
		printf("avl returned null\n");
	}
}

void TraverseTree( struct WEIGHTED_LINK *link, int tab )
{
	int count;
	for( count = 0; count<tab; count++ )
		printf("   ");

	if( link != NULL )
	{
		printf("%ld\n", link->Weight);

		TraverseTree( link->Left, tab+1 );
		TraverseTree( link->Right, tab+1 );
	}
	else
	{
		printf("null node\n");
	}
}

void DumpAvl( struct AVL_TREE * avl )
{
	printf("dumping avl\n");
	struct ELEMENT * head = (struct ELEMENT *) avl->Head;

	if( head == NULL )
		printf("empty avl\n");
	else
		TraverseTree( (struct WEIGHTED_LINK *) head, 0 );
	printf("done\n\n");
}

int main()
{
	AvlInit( & avl );
	int data;
	int temp;
	do
	{
		printf("enter a data value:\n\
				0 to exit\n\
				1 to remove a node\n\
				2 to print the data structures\n\
				or a larger number to add a node.\n");
		scanf("%d", &data );
		if( data == 0 )
			return 0;
		else if( data == 1 )
		{
			printf("which node do you want removed.\n");
			scanf("%d", &temp );
			Remove( temp );
		}
		else if( data == 2 )
		{
			DumpAvl( &avl );
		}
		else
		{
			Add(data);
		}
	}while( data != 0 );
	return 0;
}
