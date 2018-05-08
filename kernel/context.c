#include"context.h"
#include"isr.h"
#include"watchdog.h"
#include"time.h"
#include"panic.h"

/*
 * Context Unit:
 * The context unit helps deal with thread context (i.e. stacks).
 */

/*
 * When a thread is first started, this funciton is called.
 */
STACK_INIT_ROUTINE ContextBootstrap;
void ContextBootstrap(void * arg)
{
        struct CONTEXT * context = arg;

        //Here is where we end up if the kernel context switches to a new thread.
        //i.e. this is the same state as the line after HalContextSwitch.
        IsrEnable(IRQ_LEVEL_MAX);

        context->Main(context->MainArg);
        //Should never return.
        KernelPanic();
}

/*
 * Sets up a context for a future thread or co-routine.
 */
void ContextInit( struct CONTEXT * context, char * Pointer, COUNT Size, STACK_INIT_ROUTINE Foo, void * Arg)
{
#ifdef DEBUG
        int cur;
#endif

#ifdef DEBUG
        //Set up the stack boundry.
        CounterInit(&context->TimesRun);
        CounterInit(&context->TimesSwitched);
        context->LastRanTime = 0;
        context->LastSelectedTime = 0;

#endif

        //initialize stack
        if( Size != 0 )
        {
#ifdef DEBUG
                context->High = (char *) (Pointer + Size);
                context->Low = Pointer;
                //Write pattern over stack so we can expose
                //variable initialization errors.
                //TODO for loop over Pointer with count Size.
                for(cur=0; cur<Size; cur++)
                        Pointer[cur] = 0xaa;
#endif
                //Populate regular stack
                context->Main = Foo;
                context->MainArg = Arg;
                HalCreateStackFrame( &context->MachineState, Pointer, Size, ContextBootstrap, context);
        }
        else
        {
#ifdef DEBUG
                context->High = (char*) -1;
                context->Low = 0;
#endif
                //Populate stack for idle thread (machine's start thread).
                HalGetInitialStackFrame( &context->MachineState );

        }
}

/*
 * Performs a context switch from one MACHINE_CONTEXT (thread) to another.
 */
void ContextSwitch(struct CONTEXT * oldStack, struct CONTEXT * newStack)
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
                HalContextSwitch(&oldStack->MachineState, &newStack->MachineState);

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

