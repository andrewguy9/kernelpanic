#include"critinterrupt.h"
#include"../utils/atomiclist.h"
#include"hal.h"

/*
 * Crit Interrupt Unit Description
 * The interrupt unit provides additional control of the interrupt flag.
 *
 * Calls to CritInterruptDisable/CritInterruptEnable allow functions to nest 
 * disable/enable pairings so that we don't have to track all code paths 
 * around flag state changes.
 *
 * CritInterruptLevel is a count used to track the number of disables, or in other
 * words, the number of enables before crit interrupts will be allowed.
 * When CritInterruptLevel == 0, then crit interrupts should be allowed.
 * When CritInterruptLevel > 0, then crit interrupts will be disabled.
 *
 */

//
//Prototypes
//

void CritInterrupt();

//
//Crit Interrupt Variables
//

struct ATOMIC_LIST CritInterruptHandlerList;

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void CritInterruptStartup()
{
        AtomicListInit( & CritInterruptHandlerList );
        HalRegisterIsrHandler( CritInterrupt, (void *) HAL_ISR_CRIT, IRQ_LEVEL_CRIT );
}

void CritInterrupt()
{
        struct ATOMIC_LIST_LINK * link;
        struct HANDLER_OBJECT * handler;
        BOOL isComplete;
        HANDLER_FUNCTION * func;

        CritInterruptIncrement();

        //TODO TOO MUCH DUPLICATION
        while( (link = AtomicListPop(&CritInterruptHandlerList)) ) {
                handler = BASE_OBJECT(
                                link,
                                struct HANDLER_OBJECT,
                                Link );

                HandlerRun( handler );
                func = handler->Function;
                isComplete = func( handler );

                if(isComplete) {
                        HandlerFinish( handler );
                }
        }

        CritInterruptDecrement();
}

void CritInterruptRegisterHandler(
                struct HANDLER_OBJECT * handler,
                HANDLER_FUNCTION foo,
                void * context )
{

        handler->Function = foo;
        handler->Context = context;

        HandlerRegister( handler );

        AtomicListPush( &handler->Link.AtomicListLink,
                        &CritInterruptHandlerList );

        HalRaiseInterrupt(IRQ_LEVEL_CRIT);
}


