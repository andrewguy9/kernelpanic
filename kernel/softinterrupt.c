#include"softinterrupt.h"
#include"critinterrupt.h"
#include"utils/atomiclist.h"
#include"hal.h"

/*
 * SoftInterrupt Unit Description
 * The SoftInterrupt unit provides additional control of the SoftInterrupt flag.
 *
 * Calls to SoftInterruptDisable/SoftInterruptEnable allow functions to nest
 * disable/enable pairings so that we don't have to track all code paths 
 * around flag state changes.
 *
 * SoftInterruptLevel is a count used to track the number of disables, or in other
 * words, the number of enables before SoftInterrupts will be allowed.
 * When SoftInterruptLevel == 0, then interrupts should be allowed.
 * When SoftInterruptLevel > 0, then SoftInterrupt will be disabled.
 *
 */

//
//Prototypes
//

ISR_HANDLER SoftInterrupt;

//
//SoftInterrupt Variables
//

struct ATOMIC_LIST SoftInterruptHandlerList;

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void SoftInterruptStartup()
{
        AtomicListInit( &SoftInterruptHandlerList );
        IsrRegisterHandler( SoftInterrupt, (void *) HAL_ISR_SOFT, IRQ_LEVEL_SOFT );
}

void SoftInterrupt()
{
        struct ATOMIC_LIST_LINK * link;
        struct HANDLER_OBJECT * handler;
        _Bool isComplete;
        HANDLER_FUNCTION * func;

        //TODO TOO MUCH DUPLICATION.
        while( (link = AtomicListPop(&SoftInterruptHandlerList)) ) {
                handler = container_of(
                                link,
                                struct HANDLER_OBJECT,
                                Link );

                HandlerRun( handler );
                func = handler->Function;
                isComplete = func( handler );

                if (isComplete) {
                        HandlerFinish( handler );
                }

        }
}

void SoftInterruptRegisterHandler(
                struct HANDLER_OBJECT * handler,
                HANDLER_FUNCTION foo,
                void * context )
{
        handler->Function = foo;
        handler->Context = context;

        HandlerRegister( handler );

        AtomicListPush( &handler->Link.AtomicListLink,
                        &SoftInterruptHandlerList );

        HalRaiseInterrupt(IRQ_LEVEL_SOFT);
}


