#include"../kernel/hal.h"
#include"../kernel/thread.h"

#include<sys/time.h>
#include<string.h>
#include<signal.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

//-----------------------------------------------------------------------------
//-------------------------- GLOBALS ------------------------------------------
//-----------------------------------------------------------------------------

//
//Watchdog Variables
//

unsigned int HalWatchDogTimeout;
struct itimerval WatchdogInterval;

//
//Stack Management
//

STACK_INIT_ROUTINE * StackInitRoutine;

struct MACHINE_CONTEXT * halTempContext;
volatile BOOL halTempContextProcessed;


//
//Time Mangement
//

struct itimerval TimerInterval;

//
//IRQ Management
//

/*
 * This table establishes what handler to call and what signals to mask when a 
 * signal is delivered.
 *
 * sigaction is the handler to call. This will normally be the generic handler: HalIsrHandler.
 * sa_mask is the mask to apply.
 * sa_flags is used for special masking rules/alt stack settings.
 */
struct sigaction HalIrqTable[IRQ_LEVEL_COUNT];

/*
 * HalIsrHandler uses this table to call the user specified handler.
 */
ISR_HANDLER HalIsrJumpTable[IRQ_LEVEL_COUNT];

/*
 * HalIsrHandler uses this table to go from a signal to an IRQ.
 *
 * Key - IRQ level.
 * Value - Signal Number.
 *
 * HalIsrHandler will scan from the start of the array to the end.
 * When it finds the same signal number, then that index is the index it should call
 * from the HalIsrJumpTable.
 */
INDEX HalIrqToSignal[IRQ_LEVEL_COUNT];

//Create a mask for debugging
#ifdef DEBUG
sigset_t HalCurrrentIrqMask;
BOOL HalCurrrentIrqMaskValid;
#endif //DEBUG

//-----------------------------------------------------------------------------
//--------------------------- HELPER PROTOTYPES -------------------------------
//-----------------------------------------------------------------------------

//
//Watchdog
//

//
//Stack Mangement
//

void HalStackTrampoline( int SignalNumber );

//
//Time Mangement
//

//
//IRQ Management
//

#ifdef DEBUG
void HalUpdateIsrDebugInfo();
void HalInvalidateIsrDebugInfo();
#endif
void HalClearSignals();
void HalBlockSignal( void * which );

//-----------------------------------------------------------------------------
//------------------------- HELPER FUNCTIONS ----------------------------------
//-----------------------------------------------------------------------------

//
//Watchdog
//


//
//Stack Mangement
//

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

//
//Time Mangement
//

//
//IRQ Management
//

/*
 * All signals call this routine. 
 * This routine then calls the appropriate ISR Handler.
 */
void HalIsrHandler( int SignalNumber ) 
{
	INDEX index;
	enum IRQ_LEVEL irq;

#ifdef DEBUG
	HalUpdateIsrDebugInfo();
#endif

	//We dont know which irq is associated with SignalNumber, so lets find it.
	for(index = 0; index < IRQ_LEVEL_COUNT; index++) {
		if( HalIrqToSignal[index] == SignalNumber ) {
			//We found it, call the appropriate ISR.
			irq = index;
			HalIsrJumpTable[irq]();
#ifdef DEBUG
			//We are about to return into an unknown frame. 
			//I can't predict what the irq will be there.
			HalInvalidateIsrDebugInfo();
#endif
			return;
		}
	}
	
	HalPanic("Signal delivered for which no Irq was registered", SignalNumber);
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

/*
 * This is a unix specific hal function.
 * When starting up, we need to make sure that all
 * our IRQ tracking globals are initialized.
 */
void HalClearSignals()
{
	INDEX i;

	for(i=0; i < IRQ_LEVEL_COUNT; i++) {
		HalIrqTable[i].sa_handler = NULL;
		sigemptyset(&HalIrqTable[i].sa_mask);
		HalIrqTable[i].sa_flags = 0;
	}
}

/*
 * This is a unix specific hal function.
 * Some signals my be harmful to panic. 
 * This function will block them for all IRQs.
 * Must be called before all HalRegisterIsrHandler calls.
 */ 
void HalBlockSignal( void * which )
{
	INDEX i;
	INDEX signum = (INDEX) which;

	for(i=0; i < IRQ_LEVEL_COUNT; i++) {
		sigaddset(&HalIrqTable[i].sa_mask, signum);
	}
}

//-----------------------------------------------------------------------------
//-------------------------- PUBLIC FUNCTIONS ---------------------------------
//-----------------------------------------------------------------------------

//
//Hal Utilities
//

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

//
//Unit Management
//

void HalStartup()
{
}

//
//Watchdog
//

void HalWatchdogInit()
{
	//Initialize WatchdogVariables (Dont register ISR)
	HalWatchDogTimeout = 0;
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

//
//Stack Management
//

void HalContextStartup( STACK_INIT_ROUTINE * stackInitRoutine ) 
{
	StackInitRoutine = stackInitRoutine;
}

void HalCreateStackFrame( 
		struct MACHINE_CONTEXT * Context, 
		void * stack, 
		STACK_INIT_ROUTINE foo, 
		COUNT stackSize)
{
	int status;
	char * cstack = stack;
	stack_t newStack;
	sigset_t oldSet;
	sigset_t trampolineMask;
	struct sigaction switchStackAction;

	sigemptyset( &trampolineMask );
	sigaddset( &trampolineMask, HAL_ISR_TRAMPOLINE );

#ifdef DEBUG
	//Set up the stack boundry.
	Context->High = (char *) (cstack + stackSize);
	Context->Low = cstack;
#endif

	Context->Foo = foo;

	//We are about to bootstrap the new thread. Because we have to modify global
	//state here, we must make sure no interrupts occur until after we are bootstrapped.
	//We do all of this under the nose of the Isr unit.
	sigprocmask(SIG_BLOCK, &HalIrqTable[IRQ_LEVEL_MAX].sa_mask, &oldSet);

	//NOTE: We use the interrupt mask here, because we want to block all operations.
	switchStackAction.sa_handler = HalStackTrampoline;
	switchStackAction.sa_mask = HalIrqTable[IRQ_LEVEL_MAX].sa_mask;
	switchStackAction.sa_flags = SA_ONSTACK;
	sigaction(HAL_ISR_TRAMPOLINE, &switchStackAction, NULL );

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
	sigprocmask( SIG_UNBLOCK, &trampolineMask, NULL );

	raise( HAL_ISR_TRAMPOLINE );

	while( ! halTempContextProcessed );

	//Now that we have bootstrapped the new thread, lets restore the old mask.
	sigprocmask(SIG_SETMASK, &oldSet, NULL);
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
/*
 * Performs a context switch between one MACHINE_CONTEXT (thread) and another.
 * Because want to avoid having interrupts fire while the register file is in an intermidiate 
 * state, all regilar interrupts should be disabled when calling this function.
 */
void HalContextSwitch(struct MACHINE_CONTEXT * oldStack, struct MACHINE_CONTEXT * newStack)
{
	int status;
	ASSERT( HalIsIrqAtomic(IRQ_LEVEL_MAX) );

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
	}

	ASSERT( HalIsIrqAtomic(IRQ_LEVEL_MAX) );
}

//
//Time Management
//

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

void HalResetClock()
{
	//Note: On Unix we dont do anything here because 
	//the timer is already periodic, unlike the avr.
}

//
//IRQ Management
//

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

void HalRaiseSoftInterrupt()
{
	raise( HAL_ISR_SOFT );
}

//TODO IT WOULD BE NICE TO HAVE THIS DEFINED BY THE CritInterruptUnit itself.
void HalRaiseCritInterrupt()
{
	raise( HAL_ISR_CRIT );
}

void HalIsrInit() 
{
	HalClearSignals();

	//Unix Hal requires uses HAL_ISR_TRAMPOLINE to bootstrap new
	//thread stacks. We need to ensure that it is blocked by all
	//IRQ levels.
	HalBlockSignal( (void *) HAL_ISR_TRAMPOLINE );
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

	ASSERT(HalIsIrqAtomic(IRQ_LEVEL_MAX));

	for(i=level; i < IRQ_LEVEL_COUNT; i++) {
		sigaddset(&HalIrqTable[i].sa_mask, signum);
	}

	HalIrqToSignal[level] = signum;
	HalIsrJumpTable[level] = handler;
	HalIrqTable[level].sa_handler = HalIsrHandler;

	HalIsrFinalize();
	HalSetIrq(IRQ_LEVEL_MAX); 
}

void HalIsrFinalize()
{
	enum IRQ_LEVEL level;

	for(level = IRQ_LEVEL_NONE; level < IRQ_LEVEL_COUNT; level++) {
		if(HalIrqToSignal[level] != 0) {
			ASSUME( sigaction(HalIrqToSignal[level], &HalIrqTable[level], NULL), 0 );
		}
	}
}

//
//Serial Management
//

void HalSerialStartup()
{
	//TODO Fill in with rest of Serial IO Unit.
}








