#include"context.h"
#include"interrupt.h"
#include"watchdog.h"
#include"timer.h"

/*
 * Context Unit:
 * The context unit helps deal with thread context (i.e. stacks).
 */

/*
 * Sets up a machine context for a future thread.
 */
void ContextInit( struct MACHINE_CONTEXT * MachineState, char * Pointer, COUNT Size, STACK_INIT_ROUTINE Foo, INDEX debugFlag )
{
#ifdef DEBUG
	int cur;
#endif

	//Set up the watchdog flag.
	MachineState->Flag = debugFlag;
	WatchdogAddFlag( debugFlag );

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
		InterruptDisable();
		HalCreateStackFrame( MachineState, Pointer, Foo, Size );
		InterruptEnable();
	}
	else
	{
		//Populate stack for idle thread (machine's start thread).
		HalGetInitialStackFrame( MachineState );

#ifdef DEBUG
		MachineState->TimesRun = 0;
		MachineState->TimesSwitched = 0;
		MachineState->LastRanTime = 0;
		MachineState->LastSelectedTime = 0;

		MachineState->High = (char*) -1;
		MachineState->Low = 0;
#endif
	}
}

void ContextSwitch(struct MACHINE_CONTEXT * oldStack, struct MACHINE_CONTEXT * newStack)
{
#ifdef DEBUG
	TIME time = TimerGetTime();
#endif

	ASSERT( InterruptIsAtomic() );

	if( oldStack != newStack )
	{
		//The NextStack is set, so we need to context switch.
#ifdef DEBUG
		newStack->TimesSwitched++;
		newStack->TimesRun++;
		newStack->LastRanTime = time;
		newStack->LastSelectedTime = time;
#endif
		//now that the system looks like the switch has
		//happened, go ahead and do the switch.
		HalContextSwitch(oldStack, newStack);
	}
	else
	{
		//we are critical but the thread was the same,
		//so dont bother doing context switch. 
#ifdef DEBUG
		newStack->TimesRun++;
		newStack->LastRanTime = time;
#endif
	}
	ASSERT( InterruptIsAtomic() );
}

