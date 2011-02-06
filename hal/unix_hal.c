#include"../kernel/hal.h"
#include"../kernel/thread.h"

#include<sys/time.h>
#include<string.h>
#include<signal.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

//
//Unix Globals.
//

unsigned int HalWatchDogTimeout;
STACK_INIT_ROUTINE StackInitRoutine;

struct itimerval WatchdogInterval;
struct itimerval TimerInterval;

struct sigaction HalIrqTable[HAL_IRQ_MAX];

/*
                 |HAL_ISR_WATCHDOG   |HAL_ISR_TIMER   |HAL_ISR_SOFT   |HAL_ISR_CRIT   |HAL_ISR_TRAMPOLINE   |
-----------------|-------------------|----------------|---------------|---------------|---------------------|
InterruptMask    |                   |****************|***************|***************|*********************|
SoftInterruptMask|                   |                |***************|***************|*********************|
CritInterruptMask|                   |                |               |***************|*********************|
NoInterruptMask  |                   |                |               |               |*********************|
TrampolineMask   |                   |                |               |               |*********************|
-------------------------------------------------------------------------------------------------------------
*/

//Create a mask for bootstrapping new stacks. 
sigset_t TrampolineMask;

//Create a signal action for stack bootstrapping.
struct sigaction SwitchStackAction;

//Prototype for later use.
void HalWatchdogHandler( int SignalNumber );
void HalTimerHandler( int SignalNumber );
void HalSoftHandler( int SignalNumber );
void HalCritHandler( int SignalNumber );
void HalStackTrampoline( int SignalNumber );

void HalStartup()
{
	HalIsrInit();

	//Create the TrapolineMask.
	sigemptyset( &TrampolineMask );
	sigaddset( &TrampolineMask, HAL_ISR_TRAMPOLINE );

	HalBlockSignal( (void *) HAL_ISR_TRAMPOLINE );
	HalRegisterISRHandler( HalCritHandler,     (void *) HAL_ISR_CRIT,       HAL_IRQ_CRIT       );
	HalRegisterISRHandler( HalSoftHandler,     (void *) HAL_ISR_SOFT,       HAL_IRQ_SOFT       );
	HalRegisterISRHandler( HalTimerHandler,    (void *) HAL_ISR_TIMER,      HAL_IRQ_TIMER      );
	HalRegisterISRHandler( HalWatchdogHandler, (void *) HAL_ISR_WATCHDOG,   HAL_IRQ_WATCHDOG   );

	//Create the SwitchStackAction 
	//NOTE: We use the interrupt mask here, because we want to block all operations.
	SwitchStackAction.sa_handler = HalStackTrampoline;
	SwitchStackAction.sa_mask = HalIrqTable[HAL_IRQ_TIMER].sa_mask;
	SwitchStackAction.sa_flags = SA_ONSTACK;
	sigaction(HAL_ISR_TRAMPOLINE, &SwitchStackAction, NULL );

	//We start the hardware up in the InterruptSet
	//This means that no interrupts will be delivered during kernel initialization.
	HalDisableInterrupts();

	ASSERT( HalIsAtomic() );

	//Initialize WatchdogVariables (Dont register ISR)
	HalWatchDogTimeout = 0;
}

void HalInitClock()
{
	int status;

	//Set the timer interval.
	TimerInterval.it_interval.tv_sec = 0;
	TimerInterval.it_interval.tv_usec = 1000;
	TimerInterval.it_value.tv_sec = 0;
	TimerInterval.it_value.tv_usec = 1000;
	status = setitimer( ITIMER_REAL, &TimerInterval, NULL );
	ASSERT(status == 0 );
}

void HalSerialStartup()
{
	//TODO Fill in with rest of Serial IO Unit.
}

void HalContextStartup( STACK_INIT_ROUTINE stackInitRoutine ) 
{
	StackInitRoutine = stackInitRoutine;
}

#ifdef DEBUG
BOOL HalIsAtomic()
{
	//TODO HalIsAtomic needs to be rewritten.
	sigset_t curSet;
	int status;
	
	status = sigprocmask( 0, NULL, &curSet );
	ASSERT( status == 0 );

	status = sigismember( &curSet, HAL_ISR_TIMER );
	ASSERT( status != -1 );
	if( status == 1 )
	{
		//Alarm was a member of the "blocked" mask, so we are atomic.
		ASSERT( HalIsSoftAtomic() );
		return TRUE;
	}
	else
	{
		//Alarm was not a member of the "blocked" mask, so we are not atomic.
		ASSERT( status == 0 );
		return FALSE; 
	}
}

BOOL HalIsSoftAtomic()
{
	//TODO HalIsSoftAtomic needs to be rewritten.
	sigset_t curSet;
	int status;

	status = sigprocmask( 0, NULL, &curSet );
	ASSERT( status == 0 );

	status = sigismember( &curSet, HAL_ISR_SOFT );
	ASSERT( status != -1 );
	if( status == 1 )
	{
		//Alarm was a member of the "blocked" mask, so we are atomic.
		ASSERT( HalIsCritAtomic() );

		return TRUE;
	}
	else
	{
		//Alarm was not a member of the "blocked" mask, so we are not atomic.
		ASSERT( status == 0 );
		return FALSE; 
	}
}

BOOL HalIsCritAtomic()
{
	//TODO HalIsCritAtomic needs to be rewritten.
	sigset_t curSet;
	int status;

	status = sigprocmask( 0, NULL, &curSet );
	ASSERT( status == 0 );

	status = sigismember( &curSet, HAL_ISR_CRIT );
	ASSERT( status != -1 );
	if( status == 1 )
	{
		//Alarm was a member of the "blocked" mask, so we are atomic.
		return TRUE;
	}
	else
	{
		//Alarm was not a member of the "blocked" mask, so we are not atomic.
		ASSERT( status == 0 );
		return FALSE; 
	}
}

#endif //DEBUG

void HalSetIrq(INDEX irq) 
{
	sigprocmask( SIG_SETMASK, &HalIrqTable[irq].sa_mask, NULL);
}

void HalPanic(char file[], int line)
{
	printf("PANIC: %s:%d\n",file,line);
	abort(); 
}

void HalSleepProcessor()
{
	ASSERT( !HalIsAtomic() );
	pause();
}

void HalPetWatchdog( )
{
	int status;

	//Note: calling HalPetWatchdog when the watchdog is disabled
	//is safe because setting it_intertval to zero keeps the dog
	//disabled.
	WatchdogInterval.it_interval.tv_sec = 0;
	WatchdogInterval.it_interval.tv_usec = HalWatchDogTimeout * 1000;
	WatchdogInterval.it_value.tv_sec = 0;
	WatchdogInterval.it_value.tv_usec = HalWatchDogTimeout * 1000;
	status = setitimer( ITIMER_VIRTUAL, &WatchdogInterval, NULL );
	ASSERT(status == 0 );
}

void HalEnableWatchdog( int timeout )
{
	int status;

	HalWatchDogTimeout = timeout;
	// Reset the count. Beware repeated calls to HalEnableWatchdog as it will 
	// interrupt accuracy of the watchdog.
	
	//NOTE: ITIMER_VIRUTAL will decrement when the process is running.
	//This means that on unix the watchdog will not catch cases where 
	//the process is idle or sparse. 
	WatchdogInterval.it_interval.tv_sec = 0;
	WatchdogInterval.it_interval.tv_usec = HalWatchDogTimeout * 1000;
	WatchdogInterval.it_value.tv_sec = 0;
	WatchdogInterval.it_value.tv_usec = HalWatchDogTimeout * 1000;
	status = setitimer( ITIMER_VIRTUAL, &WatchdogInterval, NULL );
	ASSERT(status == 0 );
}

void HalResetClock()
{
	//Note: On Unix we dont do anything here because 
	//the timer is already periodic, unlike the avr.
}

//prototype for handler.
void TimerInterrupt();

/* Acts like the handler for a hardware
 * watchdog interrupt.
 * Panic's the kernel.
 */
void HalWatchdogHandler( int SignalNumber ) 
{
	HalPanic( "Wachdog Timeout", 0);
}

/*
 * Acts like the hardware clock.
 * Calls TimerInterrupt and updates watchdog.
 */
void HalTimerHandler( int SignalNumber )
{
	//The kernel should add this signal to the blocked list inorder to avoid 
	//nesting calls the the handler.
	//verify this.
	ASSERT( HalIsAtomic() );

	//Call the kernel's timer handler.
	TimerInterrupt();
}


//prototype for handler.
void SoftInterrupt();
void CritInterrupt();

void HalSoftHandler( int SignalNumber )
{
	SoftInterrupt();
}

/*
 * Handler for the Crit ISRs. 
 * This is just a stub.
 */
void HalCritHandler( int SignalNumber )
{
	CritInterrupt(); 
}

struct MACHINE_CONTEXT * halTempContext;
volatile BOOL halTempContextProcessed;

void HalCreateStackFrame( 
		struct MACHINE_CONTEXT * Context, 
		void * stack, 
		STACK_INIT_ROUTINE foo, 
		COUNT stackSize)
{

	int status;
	char * cstack = stack;
	stack_t newStack;

	ASSERT( HalIsAtomic() );

#ifdef DEBUG
	//Set up the stack boundry.
	Context->High = (char *) (cstack + stackSize);
	Context->Low = cstack;
#endif

	Context->Foo = foo;

	halTempContext = Context;
	halTempContextProcessed = FALSE;

	newStack.ss_sp = cstack;
	newStack.ss_size = stackSize;
	newStack.ss_flags = 0;
	status = sigaltstack( &newStack, NULL ); 
	ASSERT( status == 0 );


	//At this point we know that we are atomic. 
	//All signal types are blocked. 
	//We will unblock the Trampoine signal, and make
	//sure that it was delivered.
	sigprocmask( SIG_UNBLOCK, &TrampolineMask, NULL );

	raise( HAL_ISR_TRAMPOLINE );

	while( ! halTempContextProcessed );

	sigprocmask(SIG_BLOCK, &TrampolineMask, NULL );
}

/*
 * sigaltstack will cause this fuction to be called on an alternate stack.
 * This allows us to bootstrap new threads.
 */
void HalStackTrampoline( int SignalNumber ) 
{
	int status;
	status = _setjmp( halTempContext->Registers );

	if( status == 0 )
	{
		//Because status was 0 we know that this is the creation of
		//the stack frame. We can use the locals to construct the frame.

		halTempContextProcessed = TRUE;
		halTempContext = NULL;
		return;
	}
	else
	{
		//If we get here, then someone has jumped into a newly created thread.
		//Test to make sure we are atomic
		ASSERT( HalIsAtomic() );

		StackInitRoutine();
		
		//Returning from a function which was invoked by siglongjmp is not
		//supported. Foo should never retrun.
		HalPanic("Tried to return from StackInitRoutine!\n", 0 );
		return;
	}
}

void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context )
{
#ifdef DEBUG
	int status = _setjmp( Context->Registers );
	ASSERT( status == 0 );//We should never wake here.
#else
	_setjmp( Context->Registers );
#endif

#ifdef DEBUG
	//The stack bounderies are infinite for the initial stack.
	Context->High = (char *) -1;
	Context->Low = (char *) 0;
#endif
}

//TODO: Add stack range check.
void HalContextSwitch(struct MACHINE_CONTEXT * oldStack, struct MACHINE_CONTEXT * newStack)
{
	int status;

	//Save the stack state into old context.
	status = _setjmp( oldStack->Registers );
	if( status == 0 )
	{
		//This was the saving call to setjmp.
		_longjmp( newStack->Registers, 1 );
	}
	else
	{
		//This was the restore call started by longjmp call.
		//We have just switched into a different thread.
		ASSERT( HalIsAtomic() );
	}
}

void HalRaiseSoftInterrupt()
{
	raise( HAL_ISR_SOFT );
}

void HalRaiseCritInterrupt()
{
	raise( HAL_ISR_CRIT );
}

void HalIsrInit() 
{
	INDEX i;

	for(i=0; i < HAL_IRQ_MAX; i++) {
		HalIrqTable[i].sa_handler = NULL;
		sigemptyset(&HalIrqTable[i].sa_mask);
		HalIrqTable[i].sa_flags = 0;
	}
}

/*
 * This is a unix specific hal function.
 * Some signals my be harmful to panic. 
 * This function will block them for all IRQs.
 * Must be called before HalBlockSignal calls.
 */ 
void HalBlockSignal( void * which )
{
	INDEX i;
	INDEX signum = (INDEX) which;

	for(i=0; i<HAL_IRQ_MAX; i++) {
		sigaddset(&HalIrqTable[i].sa_mask, signum);
	}
}
/*
 * IRQ based systems typically have a reserved space in memory which 
 * serves as a jump table. The table is filled with function pointers
 * to invoke when various irq level transitions occur. Typically different
 * offsets also get a level associated with it. So that you can change the 
 * order of various devices. (so a can mask c and b can mask c.
 *
 * handler - the function pointer to invoke.
 * which - the location which indicates what hardware event happed.
 * level - what irq to assign to the hardware event.
 */
void HalRegisterISRHandler( ISR_HANDLER handler, void * which, INDEX level)
{
	INDEX i;
	INDEX signum = (INDEX) which;

	for(i=level; i<HAL_IRQ_MAX; i++) {
		sigaddset(&HalIrqTable[i].sa_mask, signum);
	}

	HalIrqTable[level].sa_handler = handler;
	ASSUME( sigaction(signum, &HalIrqTable[level], NULL), 0 );
}

