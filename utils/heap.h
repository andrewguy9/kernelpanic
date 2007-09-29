#ifndef HEAP_H
#define HEAP_H

#include"link.h"
#include"utils.h"

struct HEAP
{
	struct WEIGHTED_LINK * Head;
	COUNT Size;
};

void HeapAdd( struct WEIGHTED_LINK * node, struct HEAP * heap );

struct WEIGHTED_LINK * HeapPop( struct HEAP * heap );

WEIGHT HeapHeadWeight( struct HEAP * heap );

COUNT HeapSize( struct HEAP * heap );

void HeapInit( struct HEAP * heap );

#endif
