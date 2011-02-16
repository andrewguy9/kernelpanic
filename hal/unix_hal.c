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

struct sigaction HalIrqTable[IRQ_LEVEL_MAX + 1];

//Create a mask for bootstrapping new stacks. 
sigset_t TrampolineMask;

//Create a mask for debugging
#ifdef DEBUG
sigset_t HalCurrrentIrqMask;
BOOL HalCurrrentIrqMaskValid;
#endif //DEBUG

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
	HalRegisterIsrHandler( HalCritHandler,     (void *) HAL_ISR_CRIT,     IRQ_LEVEL_CRIT     );
	HalRegisterIsrHandler( HalSoftHandler,     (void *) HAL_ISR_SOFT,     IRQ_LEVEL_SOFT     );
	HalRegisterIsrHandler( HalTimerHandler,    (void *) HAL_ISR_TIMER,    IRQ_LEVEL_TIMER    );
	HalRegisterIsrHandler( HalWatchdogHandler, (void *) HAL_ISR_WATCHDOG, IRQ_LEVEL_WATCHDOG );

	//Create the SwitchStackAction 
	//NOTE: We use the interrupt mask here, because we want to block all operations.
	SwitchStackAction.sa_handler = HalStackTrampoline;
	SwitchStackAction.sa_mask = HalIrqTable[IRQ_LEVEL_TIMER].sa_mask;
	SwitchStackAction.sa_flags = SA_ONSTACK;
	sigaction(HAL_ISR_TRAMPOLINE, &SwitchStackAction, NULL );

	//We start the hardware up in the InterruptSet
	//This means that no interrupts will be delivered during kernel initialization.
	HalSetIrq(IRQ_LEVEL_TIMER);

	ASSERT( HalIsIrqAtomic(IRQ_LEVEL_TIMER) );

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
/*
 * Returns true if the system is running at at least IRQ level.
 */
BOOL HalIsIrqAtomic(enum IRQ_LEVEL level) 
{
	sigset_t curSet;
	int status;

	status = sigprocmask(0, NULL, &curSet);
	ASSERT(status == 0);

	HalUpdateIsrDebugInfo();

	return !((HalIrqTable[level].sa_mask ^ curSet) & HalIrqTable[level].sa_mask);
}

#endif //DEBUG

void HalSetIrq(enum IRQ_LEVEL irq) 
{
	sigprocmask( SIG_SETMASK, &HalIrqTable[irq].sa_mask, NULL);

#ifdef DEBUG
	HalUpdateIsrDebugInfo();
#endif
}

void HalPanic(char file[], int line)
{
	printf("PANIC: %s:%d\n",file,line);
	abort(); 
}

void HalSleepProcessor()
{
	ASSERT( !HalIsIrqAtomic(IRQ_LEVEL_TIMER) );
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
#ifdef DEBUG
	HalUpdateIsrDebugInfo();
#endif

	HalPanic( "Wachdog Timeout", 0);
}

/*
 * Acts like the hardware clock.
 * Calls TimerInterrupt and updates watchdog.
 */
void HalTimerHandler( int SignalNumber )
{
#ifdef DEBUG
	HalUpdateIsrDebugInfo();
#endif

	//The kernel should add this signal to the blocked list inorder to avoid 
	//nesting calls the the handler.
	//verify this.
	ASSERT( HalIsIrqAtomic(IRQ_LEVEL_TIMER) );

	//Call the kernel's timer handler.
	TimerInterrupt();

#ifdef DEBUG
	//We are about to return into an unknown frame. 
	//I can't predict what the irq will be there.
	HalInvalidateIsrDebugInfo();
#endif
}


//prototype for handler.
void SoftInterrupt();
void CritInterrupt();

void HalSoftHandler( int SignalNumber )
{
#ifdef DEBUG
	HalUpdateIsrDebugInfo();
#endif

	SoftInterrupt();

#ifdef DEBUG
	//We are about to return into an unknown frame. 
	//I can't predict what the irq will be there.
	HalInvalidateIsrDebugInfo();
#endif
}

/*
 * Handler for the Crit ISRs. 
 * This is just a stub.
 */
void HalCritHandler( int SignalNumber )
{
#ifdef DEBUG
	HalUpdateIsrDebugInfo();
#endif

	CritInterrupt(); 

#ifdef DEBUG
	//We are about to return into an unknown frame. 
	//I can't predict what the irq will be there.
	HalInvalidateIsrDebugInfo();
#endif
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

	ASSERT( HalIsIrqAtomic(IRQ_LEVEL_TIMER) );

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
		ASSERT( HalIsIrqAtomic(IRQ_LEVEL_TIMER) );

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
		ASSERT( HalIsIrqAtomic(IRQ_LEVEL_TIMER) );
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

	for(i=0; i < IRQ_LEVEL_MAX + 1; i++) {
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

	for(i=0; i<IRQ_LEVEL_MAX + 1; i++) {
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
void HalRegisterIsrHandler( ISR_HANDLER handler, void * which, enum IRQ_LEVEL level)
{
	INDEX i;
	INDEX signum = (INDEX) which;

	for(i=level; i<IRQ_LEVEL_MAX + 1; i++) {
		sigaddset(&HalIrqTable[i].sa_mask, signum);
	}

	HalIrqTable[level].sa_handler = handler;
	ASSUME( sigaction(signum, &HalIrqTable[level], NULL), 0 );
}

#ifdef DEBUG
void HalUpdateIsrDebugInfo()
{
	HalCurrrentIrqMaskValid = TRUE;
	sigprocmask(0, NULL, &HalCurrrentIrqMask);
}

void HalInvalidateIsrDebugInfo()
{
	HalCurrrentIrqMaskValid = FALSE;
}
#endif
