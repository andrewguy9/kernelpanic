#include"timer.h"
#include"../utils/utils.h"
#include"../utils/heap.h"
#include"thread.h"
#include"isr.h"
#include"softinterrupt.h"
#include"hal.h"

/*
 * Timer Unit Description:
 * The timer unit keeps time for the kernel. The current time can be queried
 * by calling TimerGetTime().
 *
 * Timers can be registered with the system by calling TimerRegister().
 * When the timer fires, the function and argument provided to TimerRegister()
 * are called as a post interrupt handler. (So interrupts will be ENABLED).
 * Its perfectly safe to have a timer re-register itself.
 */

//TODO WE SHOULD REFACTOR TIME INTO A TIME UNIT, AND HAVE TIMERS
//BE IN THIS UNIT.

//
//  Prototypes
//

void TimerInterrupt(void);
void TimerInner(TIME time);
void QueueTimers(TIME time, struct HEAP * heap);
void TimerSetNextTimer(TIME time);

//
//Unit Variables
//

//Keep track of system time.
TIME TimerLastTime;

//Keep track of timers waiting to execute.
//We protect the timer heap with IRQ_LEVEL_MAX.
struct HEAP TimerHeap1;
struct HEAP TimerHeap2;

struct HEAP * Timers;
struct HEAP * TimersOverflow;

//
//Unit Helper Routines
//

void QueueTimers(TIME time, struct HEAP * heap)
{
        //TODO TOO MUCH DUPLICATION.
        while( HeapSize( heap ) > 0 &&
                        HeapHeadWeight( heap ) <= time ) {

                struct HANDLER_OBJECT * timer = BASE_OBJECT(
                                HeapPop(  heap ),
                                struct HANDLER_OBJECT,
                                Link );

                //Mark timer as running since its dequeued.
                HandlerRun( timer );

                SoftInterruptRegisterHandler(
                                timer,
                                timer->Function,
                                timer->Context);
        }
}

/*
 * Takes expired timers off of the heap,
 * and queues them as SoftInterrupts
 * list.
 */
void TimerInner( TIME time )
{
        struct HEAP *temp;

        if ( time < TimerLastTime ) {
                //Overflow occured, drain heap

                QueueTimers(-1, Timers);
                ASSERT(HeapSize(Timers) == 0);

                //Switch Queues.

                temp = Timers;
                Timers = TimersOverflow;
                TimersOverflow = temp;
        }
        QueueTimers(time, Timers);

        //Now that we have de-queued all the fired timers,
        //lets calculate when the next hardware interrupt should be.
        TimerSetNextTimer( time );
        TimerLastTime = time;
}

void TimerSetNextTimer(TIME time)
{
        if (HeapSize(Timers) > 0) {
                TIME nextTimer = HeapHeadWeight( Timers );
                TIME delta = nextTimer - time;
                HalSetTimer(delta);
        } else {
                //If there are no timers in the Timers heap,
                //then we know that the next time to wake will be
                //an unknown number after the next overflow.
                TIME rollover = -1 - time;
                HalSetTimer(rollover + 1);
        }
}

void TimerStartup( )
{
        HalInitClock();
        TimerLastTime = HalGetTime();

        HeapInit( &TimerHeap1 );
        HeapInit( &TimerHeap2 );

        Timers = &TimerHeap1;
        TimersOverflow = &TimerHeap2;

        HalRegisterIsrHandler( TimerInterrupt, (void *) HAL_ISR_TIMER, IRQ_LEVEL_TIMER );
        TimerSetNextTimer(TimerLastTime);
}

void TimerRegister(
                struct HANDLER_OBJECT * newTimer,
                TIME wait,
                HANDLER_FUNCTION * handler,
                void * context )
{
        TIME time = HalGetTime();
        TIME timerTime = time + wait;

        IsrDisable(IRQ_LEVEL_MAX);

        //Construct timer
        HandlerRegister( newTimer );
        newTimer->Function = handler;
        newTimer->Context = context;

        //Add to heap
        if ( timerTime >= time ) {
                HeapAdd(timerTime, &newTimer->Link.WeightedLink, Timers );
        } else {
                //Overflow ocurred
                HeapAdd(timerTime, &newTimer->Link.WeightedLink, TimersOverflow);
        }

        IsrEnable(IRQ_LEVEL_MAX);
        //Because we added a new timer, we may want to wait
        //a different amount of time than previously thought.
        //Lets update the hardware countdown.
        TimerSetNextTimer(time);

}

TIME TimerGetTime()
{
        return HalGetTime();
}

void TimerInterrupt(void)
{
        //update interrupt level to represent that we are in inerrupt
        TimerIncrement();

        //reset the clock
        HalResetClock();

        //Queue Timers to run as Post Handlers.
        TimerInner( HalGetTime() );

        //Restore the interrupt level,
        TimerDecrement();
}

