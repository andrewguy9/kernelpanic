#ifndef GENERATION_H
#define GENERATION_H

#include"locking.h"
#include"handler.h"
#include"../utils/linkedlist.h"

struct GENERATION
{
        COUNT Generation;
        struct LINKED_LIST WaitingThreads;
};

struct GENERATION_CONTEXT {
        struct GENERATION * Generation;
        COUNT NewValue;
};

void GenerationInit(struct GENERATION * gen, COUNT init);
void GenerationWait(struct GENERATION * gen, COUNT value, struct LOCKING_CONTEXT * context);
void GenerationUpdate(struct GENERATION * gen, COUNT value);
void GenerationUpdateSafe(struct GENERATION * gen, COUNT value, struct GENERATION_CONTEXT * context, struct HANDLER_OBJECT * handler);

#endif
