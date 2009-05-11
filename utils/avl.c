#include"avl.h"

void AvlAdd( struct WEIGHTED_LINK * Node, struct AVL_TREE * Tree )
{
	struct WEIGHTED_LINK * parent;

	ASSERT( Node != NULL );

	Node->Left = NULL;
	Node->Right = NULL;
	parent = Tree->Head;

	if( parent == NULL ) 
	{
		Tree->Head = Node;
		Node->Parent = NULL;
	}
	else 
	{
		while( TRUE )
		{
			if( Node->Weight > parent->Weight )
			{
				if( parent->Right )
					parent = parent->Right;
				else
					break;
			}
			else 
			{
				if( parent->Left )
					parent = parent->Left;
				else
					break;
			}
		}
		
		ASSERT( parent != NULL );

		if( Node->Weight > parent->Weight )
		{
			ASSERT( parent->Right == NULL );

			parent->Right = Node;
			Node->Parent = parent;
		}
		else 
		{
			ASSERT( parent->Left == NULL );

			parent->Left = Node;
			Node->Parent = parent;

		}
	}
}

struct WEIGHTED_LINK * AvlFind( WEIGHT Weight, struct AVL_TREE * Tree )
{
	struct WEIGHTED_LINK * node = Tree->Head;

	while( node != NULL )
	{
		if( node->Weight == Weight )
			break;
		else if( Weight > node->Weight )
			node = node->Right;		
		else
			node = node->Left;
	}
	return node;
}

void AvlInit( struct AVL_TREE * Tree )
{
	Tree->Head = NULL;
}

void AvlDelete( struct WEIGHTED_LINK * node, struct AVL_TREE * Tree )
{
	struct WEIGHTED_LINK * parent;
	struct WEIGHTED_LINK * left;
	struct WEIGHTED_LINK * right;

	ASSERT( node != NULL );

	parent = node->Parent;
	left = node->Left;
	right = node->Right;

	if( !left && !right )
	{
		//Leaf node.

		//remove parent's link to node.
		if( parent )
		{
			if( parent->Left == node )
			{
				parent->Left = NULL;
			}
			else
			{
				ASSERT( parent->Right == node );
				parent->Right = NULL;
			}
		}
		else 
		{
			ASSERT( Tree->Head == node );
			Tree->Head = NULL;
		}
	}
	else if( left && right )
	{
		//Both children present.
		
		//Replace parent's link or head in tree.
		left->Parent = parent;
		if( parent ) 
		{
		   if( parent->Left == node )
		   {
			   parent->Left = left;
		   }
		   else
		   {
			   ASSERT( parent->Right == node );
			   parent->Right = left;
		   }
		}
		else
		{
			ASSERT( Tree->Head == node );
			Tree->Head = left;
		}
		//Replace right's parent link.
		struct WEIGHTED_LINK * rightMost = left;
		while( rightMost->Right != NULL )
			rightMost = rightMost->Right;
		rightMost->Right = right;
		right->Parent = rightMost;
	}	
	else if( left ) 
	{
		//replace node with left.
		left->Parent = parent;
		if( parent )
		{
			if( parent->Left == node )
			{
				parent->Left = left;
			}
			else
			{
				ASSERT( parent->Right == node );
				parent->Right = left;
			}
		}
		else
		{
			ASSERT( Tree->Head == node );
			Tree->Head = left;
		}
	}
	else
	{
		//Replace node with right.
		
		ASSERT( left && !right );
		right->Parent == parent;
		if( parent )
		{
			if( parent->Left == node )
			{
				parent->Left = right;
			}
			else
			{
				ASSERT( parent->Right = node );
				parent->Right = right;
			}
		} 
		else
		{	
			ASSERT( Tree->Head == node );
			Tree->Head = right;
		}
	}

	//Now the tree structure is consistant, zero out pointers in node.
	node->Parent = NULL;
	node->Left = NULL;
	node->Right = NULL;
}

