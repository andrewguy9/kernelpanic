#ifndef AVL_H
#define AVL_H

#include"link.h"

struct AVL_TREE
{
    struct WEIGHTED_LINK * Head;
};

void AvlAdd( WEIGHT weight, struct WEIGHTED_LINK * Node, struct AVL_TREE * Tree );
struct WEIGHTED_LINK * AvlFind( WEIGHT Weight, struct AVL_TREE * Tree );
void AvlInit( struct AVL_TREE * Tree );
void AvlDelete( struct WEIGHTED_LINK * node, struct AVL_TREE * Tree );

#endif
