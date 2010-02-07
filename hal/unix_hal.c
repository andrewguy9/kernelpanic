#include<sys/time.h>
#include<string.h>
#include<signal.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

//
//Define signal to kernel interrupt mappings.
//

#define HAL_SIGNAL_TRAMPOLINE SIGINFO
#define HAL_SIGNAL_TIMER SIGALRM
#define HAL_SIGNAL_SOFT SIGUSR1
#define HAL_SIGNAL_CRIT SIGUSR2

//
//Unix Globals.
//

char HalWatchdogMask;
BOOL HalWatchdogOn;
unsigned int HalWatchDogFrequency;
unsigned int HalWatchdogCount;

struct itimerval TimerInterval;

/*
                |SIGALRM|SIGUSR1|SIGUSR2|SIGINFO|
-------------------------------------------------
InterruptMask    |*******|*******|*******|*******|
SoftInterruptMask|       |*******|*******|*******|
CritInterruptMask|       |       |*******|*******|
NoInterruptMask  |       |       |       |*******|
TrampolineMask   |       |       |       |*******|
-------------------------------------------------
*/

//Masks are used to disable various interrupt types.
//If a signal is in the mask it cannot be delivered
//once that mask is applied.
sigset_t InterruptMask;
sigset_t SoftInterruptMask;
sigset_t CritInterruptMask;
sigset_t NoInterruptMask;
sigset_t TrampolineMask;

//Timer Action Storage.
struct sigaction TimerAction;
struct sigaction SoftAction;
struct sigaction CritAction;
struct sigaction SwitchStackAction;

//Prototype for later use.
void HalUnixTimer( int SignalNumber );
void SoftHandler( int SignalNumber );
void CritHandler( int SignalNumber );
void HalStackTrampoline( int SignalNumber );

void HalStartup()
{
	int status;

	/*
	 * Create the masks:
	 * The InterruptMasks are signal masks which prevent
	 * different kinds of signals from being delivered at different times.
	 */

	//Create the InterruptMask.
	status = sigemptyset( &InterruptMask );
	ASSERT( status == 0 );
	status = sigaddset( &InterruptMask, HAL_SIGNAL_TIMER );
	ASSERT( status == 0 );
	status = sigaddset( &InterruptMask, HAL_SIGNAL_SOFT );
	ASSERT( status == 0 );
	status = sigaddset( &InterruptMask, HAL_SIGNAL_CRIT );
	ASSERT( status == 0 );
	status = sigaddset( &InterruptMask, HAL_SIGNAL_TRAMPOLINE );
	ASSERT( status == 0 );

	//Create the SoftInterruptMask.
	status = sigemptyset( &SoftInterruptMask );
	ASSERT( status == 0 );
	status = sigaddset( &SoftInterruptMask, HAL_SIGNAL_SOFT );
	ASSERT( status == 0 );
	status = sigaddset( &SoftInterruptMask, HAL_SIGNAL_CRIT );
	ASSERT( status == 0 );
	status = sigaddset( &SoftInterruptMask, HAL_SIGNAL_TRAMPOLINE );
	ASSERT( status == 0 );

	//Create the enable CritInterruptMask
	status = sigemptyset( &CritInterruptMask );
	ASSERT( status == 0 );
	status = sigaddset( &CritInterruptMask, HAL_SIGNAL_CRIT );
	ASSERT( status == 0 );
	status = sigaddset( &CritInterruptMask, HAL_SIGNAL_TRAMPOLINE );
	ASSERT( status == 0 );

	//Create the NoInterruptMask.
	status = sigemptyset( &NoInterruptMask );
	ASSERT( status == 0 );
	status = sigaddset( &NoInterruptMask, HAL_SIGNAL_TRAMPOLINE );
	ASSERT( status == 0 );

	//Create the TrapolineMask.
	//TODO: WE NEED TO OVERHAUL THE TRAMPOLINE MASK.
	status = sigemptyset( &TrampolineMask );
	ASSERT( status == 0 );
	status = sigaddset( &TrampolineMask, HAL_SIGNAL_TRAMPOLINE );
	ASSERT( status == 0 );

	/*
	 * Create the actions:
	 * The Actions are calls to register for various signals
	 */

	//Create the timer action.
	TimerAction.sa_handler = HalUnixTimer;
	TimerAction.sa_mask = InterruptMask;
	TimerAction.sa_flags = 0;
	status = sigaction( HAL_SIGNAL_TIMER, &TimerAction, NULL );
	ASSERT( status == 0 );	


	//Create the soft action.
	SoftAction.sa_handler = SoftHandler;
	SoftAction.sa_mask = SoftInterruptMask;
	SoftAction.sa_flags = 0;
	status = sigaction( HAL_SIGNAL_SOFT, &SoftAction, NULL );
	ASSERT( status == 0 );	

	//Create the crit action.
	CritAction.sa_handler = CritHandler;
	CritAction.sa_mask = CritInterruptMask;
	CritAction.sa_flags = 0;
	status = sigaction( HAL_SIGNAL_CRIT, &CritAction, NULL );
	ASSERT( status == 0 );	

	//Create the SwitchStackAction 
	//NOTE: We use the interrupt mask here, because we want to block all operations.
	SwitchStackAction.sa_handler = HalStackTrampoline;
	SwitchStackAction.sa_mask = InterruptMask;
	SwitchStackAction.sa_flags = SA_ONSTACK;
	status = sigaction(HAL_SIGNAL_TRAMPOLINE, &SwitchStackAction, NULL );

	//We start the hardware up in the InterruptSet
	//This means that no interrupts will be delivered during kernel initialization.
	status = sigprocmask( SIG_SETMASK, &InterruptMask, NULL );
	ASSERT( status == 0 );

	ASSERT( HalIsAtomic() );

	//Set up the watchdog.
	HalWatchdogOn = FALSE;
	HalWatchdogMask = 0;
	HalWatchDogFrequency = 0;
	HalWatchdogCount = 0;
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
	//TODO
}

#ifdef DEBUG
BOOL HalIsAtomic()
{
	sigset_t curSet;
	int status;
	
	status = sigprocmask( 0, NULL, &curSet );
	ASSERT( status == 0 );

	status = sigismember( &curSet, HAL_SIGNAL_TIMER );
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
	sigset_t curSet;
	int status;

	status = sigprocmask( 0, NULL, &curSet );
	ASSERT( status == 0 );

	status = sigismember( &curSet, HAL_SIGNAL_SOFT );
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
	sigset_t curSet;
	int status;

	status = sigprocmask( 0, NULL, &curSet );
	ASSERT( status == 0 );

	status = sigismember( &curSet, HAL_SIGNAL_CRIT );
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

void HalDisableInterrupts()
{
	int status;

	status = sigprocmask( SIG_SETMASK, &InterruptMask, NULL ); 
	ASSERT( status == 0 );

	ASSERT( HalIsAtomic() );
}

void HalDisableSoftInterrupts()
{
	int status;

	status = sigprocmask( SIG_SETMASK, &SoftInterruptMask, NULL ); 
	ASSERT( status == 0 );

	ASSERT( HalIsSoftAtomic() );
}

void HalDisableCritInterrupts()
{
	int status;

	status = sigprocmask( SIG_SETMASK, &CritInterruptMask, NULL ); 
	ASSERT( status == 0 );
}

void HalEnableInterrupts()
{
	int status;

	status = sigprocmask( SIG_SETMASK, &NoInterruptMask, NULL );
	ASSERT( status == 0 );
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
	HalWatchdogCount = 0;	
}

void HalEnableWatchdog( int frequency )
{
	HalWatchdogOn = TRUE;
	HalWatchDogFrequency = frequency;
	// Reset the count. Beware repeated calls to HalEnableWatchdog as it will 
	// interrupt accuracy of the watchdog.
	HalWatchdogCount = 0;
}

void HalResetClock()
{
	//TODO NO NOTHING FOR NOW SINCE ALARM IS OUR GLOBAL FLAG.
	//and the alarm is already periodic. (unlike the avr).
}

//prototype for handler.
void TimerInterrupt();

/*
 * Acts like the hardware clock.
 * Calls TimerInterrupt and updates watchdog.
 */
void HalUnixTimer( int SignalNumber )
{
	//The kernel should add this signal to the blocked list inorder to avoid 
	//nesting calls the the handler.
	//verify this.
	ASSERT( HalIsAtomic() );

	//Call the kernel's timer handler.
	TimerInterrupt();

	//TODO IF POSSIBLE, MOVE WATCHDOG INTO OWN TIMER.
	//Run the watchdog check
	if( HalWatchdogOn )
	{
		HalWatchdogCount ++;

		if( HalWatchdogCount > HalWatchDogFrequency )
		{
			//The time for a match has expried. Panic!!!
			HalPanic("Watchdog Timeout", 0 );
		}
	}
}


//prototype for handler.
void SoftInterrupt();
void CritInterrupt();

void SoftHandler( int SignalNumber )
{
	SoftInterrupt();
}

/*
 * Handler for the Crit ISRs. 
 * This is just a stub.
 */
void CritHandler( int SignalNumber )
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


		//TODO CHANGE THIS MASKING OPERATION.
		status = sigprocmask( SIG_UNBLOCK, &TrampolineMask, NULL );
		ASSERT( status == 0 );

		raise( HAL_SIGNAL_TRAMPOLINE );

		while( ! halTempContextProcessed );

		status = sigprocmask(SIG_BLOCK, &TrampolineMask, NULL );
		ASSERT( status == 0 );
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

		//Our local variables are missing, but we know ActiveThread
		//is the current thread which we can to run. We can get main from there.
		ActiveStack->Foo();
		
		//Returning from a function which was invoked by siglongjmp is not
		//supported. Foo should never retrun.
		HalPanic("Tried to return from ActiveStack->Foo()\n", 0 );
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
void HalContextSwitch( )
{
	int status;
	struct MACHINE_CONTEXT * oldContext = ActiveStack;
	struct MACHINE_CONTEXT * newContext = NextStack;

	ActiveStack = NextStack;
	NextStack = NULL;

	//Save the stack state into old context.
	status = _setjmp( oldContext->Registers );
	if( status == 0 )
	{
		//This was the saving call to setjmp.
		_longjmp( newContext->Registers, 1 );
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
	raise( HAL_SIGNAL_SOFT );
}

void HalRaiseCritInterrupt()
{
	raise( HAL_SIGNAL_CRIT );
}
