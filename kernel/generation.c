#include"generation.h"
#include"critinterrupt.h"

/*
 * Generation Unit Description
 * Blocks all threads whoose generatio count matches the locks generation count.
 */

/*
 * Updates a generation with a new value from a high IRQ.
 */
_Bool GenerationUpdateHandler(struct HANDLER_OBJECT * HandlerObj)
{
        struct GENERATION_CONTEXT * context = HandlerObj->Context;
        GenerationUpdate(context->Generation, context->NewValue);
        return true;
}

/*
 * Public Functions.
 */
void GenerationInit(struct GENERATION * gen, COUNT init)
{
        gen->Generation = init;
        LinkedListInit(&gen->WaitingThreads);
}

// Causes conext to wait if his generation count is different than gen.
void GenerationWait(struct GENERATION * gen, COUNT value, struct LOCKING_CONTEXT * context)
{
        LockingStart();

        if (gen->Generation == value) {
                union LINK * link = LockingBlock( NULL, context );
                LinkedListEnqueue( &link->LinkedListLink,
                                &gen->WaitingThreads );
        } else {
                LockingAcquire( context );
        }

        LockingEnd();
}

// Updates gen to be value. Unblocks everyone behind it.
void GenerationUpdate(struct GENERATION * gen, COUNT value)
{
        struct LOCKING_CONTEXT * context;

        LockingStart();

        gen->Generation = value;
        while (! LinkedListIsEmpty( &gen->WaitingThreads) ) {
                context = container_of(
                                LinkedListPop(&gen->WaitingThreads),
                                struct LOCKING_CONTEXT,
                                Link);
                LockingAcquire( context );
        }

        LockingEnd();
}

// If at a high IRQ use this to schedule an update to gen via the
// crit handler mechanism.
void GenerationUpdateSafe(
                struct GENERATION * gen,
                COUNT value,
                struct GENERATION_CONTEXT * context,
                struct HANDLER_OBJECT * handler)
{
        context->Generation = gen;
        context->NewValue = value;
        CritInterruptRegisterHandler(handler, &GenerationUpdateHandler, context);
}
