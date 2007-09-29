#ifndef LINK_H
#define LINK_H

typedef unsigned long int WEIGHT;

struct LINKED_LIST_LINK 
{
	struct LINKED_LIST_LINK * Next;
	struct LINKED_LIST_LINK * Prev;
};

struct WEIGHTED_LINK
{
	struct WEIGHTED_LINK * Left;
	struct WEIGHTED_LINK * Right;
	struct WEIGHTED_LINK * Parent;
	WEIGHT Weight;
};

union LINK
{
	struct LINKED_LIST_LINK LinkedListLink;
	struct WEIGHTED_LINK WeightedLink;
};

#endif
