#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include"link.h"
#include"utils.h"

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

#define LINKED_LIST_FOR_EACH(item, T, field) \
  for (T * item = BASE_OBJECT(list->Head, T, field), \
      * last = NULL, \
      * head = BASE_OBJECT(list->Head, T, field); \
      list->Head && (item != head || last == NULL); \
      last = item, \
      item = BASE_OBJECT(item->field.LinkedListLink.Next, T, field))

#endif
