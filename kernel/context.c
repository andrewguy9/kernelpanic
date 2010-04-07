#include"context.h"
#include"interrupt.h"
#include"watchdog.h"
#include"timer.h"

/*
 * Context Unit:
 * TODO REWRITE THE CONTEXT UNIT TO BE A STACK MANAGEMENT UNIT.
 * TODO WE SHOULD MOVE BACK TO ActiveThread and NextThread pointers.
 * The context unit manages "what context is on the stack".
 * In other words the context unit allows for critical sections
 * by preventing context switches. It is solely responsible for
 * calling the HAL's context switch code.
 *
 * The context unit manages the context switch lock in similarly
 * to how the interrupt unit manages the interrupt flag. 
 *
 * The rule is that you lock the stack by entering a critical
 * section by calling CritInterruptStart() 
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
		//TODO ADD PARAMS
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

