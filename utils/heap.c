#include"heap.h"

/*
 * A Heap implementation based on using WEIGHTED_LINK nodes. 
 */

void HeapPromote( struct WEIGHTED_LINK * node )
{
	//NOTE: this function does not protect the heap->Head value. 
	//You must fix the head value after calling this function.

	//Can not promote top node!
	ASSERT( node->Parent != NULL );

	struct WEIGHTED_LINK * parent = node->Parent;
	struct WEIGHTED_LINK * grandParent = parent->Parent;
	struct WEIGHTED_LINK * nodeLeft = node->Left;
	struct WEIGHTED_LINK * nodeRight = node->Right;
	struct WEIGHTED_LINK * sibling = ( parent->Left != node ? parent->Left : parent->Right );

	//make grandParent point to node
	if( grandParent != NULL )
	{
		if( grandParent->Left == parent )
			grandParent->Left = node;
		else
			grandParent->Right = node;
	}

	//make node point to grandParent
	node->Parent = grandParent;

	//make parent and sibling point to node as new parent.
	parent->Parent = node;
	if( sibling != NULL )
		sibling->Parent = node;

	//make node add parent and sibling as children.
	if( parent->Left == node )
	{
		node->Left = parent;
		node->Right = sibling;
	}
	else
	{
		node->Left = sibling;
		node->Right = parent;
	}

	//make parent point to nodeLeft and nodeRight as children.
	parent->Left = nodeLeft;
	parent->Right = nodeRight;

	//make nodeLeft and nodeRight point to parent
	if( nodeLeft != NULL )
		nodeLeft->Parent = parent;
	if( nodeRight != NULL )
		nodeRight->Parent = parent;
}

struct WEIGHTED_LINK * HeapSmallest( struct WEIGHTED_LINK *n1, struct WEIGHTED_LINK *n2, struct WEIGHTED_LINK *n3 )
{
	WEIGHT w1;
	if( n1 != NULL )
		w1 = n1->Weight;
	else
		w1 = -1;

	WEIGHT w2;
	if( n2 != NULL )
		w2 = n2->Weight;
	else
		w2 = -1;

	WEIGHT w3;
	if( n3 != NULL )
		w3 = n3->Weight;
	else
		w3 = -1;

	if( w1 <= w2 && w1 <= w3 )
		return n1;
	if( w2 <= w1 && w2 <= w3 )
		return n2;
	if( w3 <= w1 && w3 <= w2 )
		return n3;
	else 
		return NULL;
}

struct WEIGHTED_LINK * HeapFindElement( INDEX index, struct HEAP * heap )
{

	//Make sure that the index is in bounds.
	ASSERT( index <= heap->Size && index != 0 );

	//Find the number of steps go from head to index.
	COUNT steps=0;
	INDEX tmp=index;
	for( ; tmp != 1; tmp/=2 )
		steps++;

	if( steps == 0 )//we should not step...
		return heap->Head;

	//start at head and decend to the desired index.
	struct WEIGHTED_LINK * cur = heap->Head;
	unsigned int mask;
	for(mask = ( 1 << steps )/2 ; mask > 0; mask/=2 )
	{
		if( mask & index )
		{//true, the bit is 1, go right
			cur = cur->Right;
		}
		else
		{//false, bit must have been 0.
			cur = cur->Left;
		}
	}

	return cur;
}

void HeapAdd(WEIGHT weight, struct WEIGHTED_LINK * node, struct HEAP * heap )
{

	struct WEIGHTED_LINK * parent;
	
	//set the weight so we can start to percolate.
	node->Weight = weight;

	//node will be a leaf, so set pointers to null.
	node->Left = NULL;
	node->Right = NULL;

	//Increase the heaps size to represent node.
	heap->Size++;
	if( heap->Size == 1 )
	{//node is the first element.
		heap->Head = node;
		//set up node with no parent since its the root.
		node->Parent = NULL;
	}
	else
	{//add at tail and percolate up.
		//find parent of node and add.
		parent = HeapFindElement( heap->Size/2, heap );

		//The parent is gauranteed to have a spot for node.
		ASSERT( parent->Left == NULL || parent->Right == NULL );

		if( parent->Left == NULL )
			parent->Left = node;
		else
			parent->Right = node;
		node->Parent = parent;
		//now percolate node to the top...
		while( node->Parent != NULL && node->Weight < node->Parent->Weight )
		{//percolate...
			HeapPromote( node );
		}
		//if node has no parent now, then it must be root.
		if( node->Parent == NULL )
			heap->Head = node;
	}
	return ;
}

struct WEIGHTED_LINK * HeapPop( struct HEAP * heap )
{
	if( heap->Size == 0 )
		return NULL;


	//we need an element to replace head.	
	struct WEIGHTED_LINK * tail = HeapFindElement( heap->Size, heap );
	//make sure tail is a leaf.
	ASSERT( tail != NULL );
	ASSERT( tail->Left == NULL && tail->Right == NULL);
	//remove tail from heap (from its parent)
	if( tail->Parent != NULL )
	{//if parent exists: i.e. we are in middle of heap
		if( tail->Parent->Right == tail )
			tail->Parent->Right = NULL;
		else
			tail->Parent->Left = NULL;
	}
	else
	{//if tail has no parent, it is head (only node) We should complete now.
		heap->Size = 0;
		heap->Head = NULL;
		return tail;
	}
		
	//save a pointer to head
	struct WEIGHTED_LINK * popped = heap->Head;

	//put tail in place of head.
	heap->Size--;
	heap->Head = tail;
	tail->Parent = NULL;
	tail->Left = popped->Left;
	tail->Right = popped->Right;
	//make children point at tail as parent.
	if( tail->Left != NULL )
		tail->Left->Parent = tail;
	if( tail->Right != NULL )
		tail->Right->Parent = tail;

	//percolate tail down
	struct WEIGHTED_LINK * smallest;
	for( smallest = HeapSmallest( tail, tail->Left, tail->Right); 
			smallest != tail; 
			smallest = HeapSmallest( tail, tail->Left, tail->Right) )
	{
		HeapPromote( smallest );
		if( smallest->Parent == NULL )//NOTE: this only needs to be checked the first time.
			heap->Head = smallest;
	}

	//return the popped value.
	return popped;
}

WEIGHT HeapHeadWeight( struct HEAP * heap )
{
	return heap->Head->Weight;
}

void HeapInit( struct HEAP * heap )
{
	heap->Head = NULL;
	heap->Size = 0;
}

COUNT HeapSize( struct HEAP * heap )
{
	return heap->Size;
}
