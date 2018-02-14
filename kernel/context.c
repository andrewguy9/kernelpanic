#include"context.h"
#include"isr.h"
#include"watchdog.h"
#include"time.h"

/*
 * Context Unit:
 * The context unit helps deal with thread context (i.e. stacks).
 */

//This variable is used to hold the function to call when a new context has been switched into
//for the first time.
STACK_INIT_ROUTINE * ContextHandoff;//XXX this is not guarded.

/*
 * When a thread is first started, this funciton is called.
 */
STACK_INIT_ROUTINE ContextBootstrap;
void ContextBootstrap()
{

        //Here is where we end up if the kernel context switches to a new thread.
        //i.e. this is the same state as the line after HalContextSwitch.
        IsrEnable(IRQ_LEVEL_MAX);

        ContextHandoff();
}

/*
 * Sets up a machine context for a future thread.
 */
void ContextInit( struct MACHINE_CONTEXT * MachineState, char * Pointer, COUNT Size, STACK_INIT_ROUTINE Foo)
{
#ifdef DEBUG
        int cur;
#endif

        //initialize stack
        if( Size != 0 )
        {
#ifdef DEBUG
                //Write pattern over stack so we can expose
                //variable initialization errors.
                for(cur=0; cur<Size; cur++)
                        Pointer[cur] = 0xaa;
#endif
                //Populate regular stack
                ContextHandoff = Foo;
                HalCreateStackFrame( MachineState, Pointer, Size, ContextBootstrap );
        }
        else
        {
                //Populate stack for idle thread (machine's start thread).
                HalGetInitialStackFrame( MachineState );

#ifdef DEBUG
                CounterInit(&MachineState->TimesRun);
                CounterInit(&MachineState->TimesSwitched);
                MachineState->LastRanTime = 0;
                MachineState->LastSelectedTime = 0;

                MachineState->High = (char*) -1;
                MachineState->Low = 0;
#endif
        }
}

/*
 * Performs a context switch from one MACHINE_CONTEXT (thread) to another.
 */
void ContextSwitch(struct MACHINE_CONTEXT * oldStack, struct MACHINE_CONTEXT * newStack)
{
#ifdef DEBUG
        TIME time = TimeGet();
#endif

        if( oldStack != newStack ) {
                //The NextStack is set, so we need to context switch.
#ifdef DEBUG
                CounterAdd(&newStack->TimesSwitched, 1);
                CounterAdd(&newStack->TimesRun, 1);
                newStack->LastRanTime = time;
                newStack->LastSelectedTime = time;
#endif

                //The Hal requires that no interrupts fire during the switch.
                IsrDisable(IRQ_LEVEL_MAX);

                //now that the system looks like the switch has
                //happened, go ahead and do the switch.
                //NOTE: If you change anything below here, you have to update ContextBootstrap.
                HalContextSwitch(oldStack, newStack);

                IsrEnable(IRQ_LEVEL_MAX);
        } else {
                //we are critical but the thread was the same,
                //so dont bother doing context switch.
#ifdef DEBUG
                CounterAdd(&newStack->TimesRun, 1);
                newStack->LastRanTime = time;
#endif
        }
}

