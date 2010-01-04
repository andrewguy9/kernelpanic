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

//Used for reference.
sigset_t EmptySet;
sigset_t TimerSet;
sigset_t SoftSet;
sigset_t CritSet;
sigset_t TrampolineSet;

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

	//Create the empty set.
	status = sigemptyset( &EmptySet );
	ASSERT( status == 0 );
	status = sigaddset( &EmptySet, HAL_SIGNAL_TRAMPOLINE );
	ASSERT( status == 0 );

	//Create the timer set.
	status = sigemptyset( &TimerSet );
	ASSERT( status == 0 );
	status = sigaddset( &TimerSet, HAL_SIGNAL_TIMER );
	ASSERT( status == 0 );
	status = sigaddset( &TimerSet, HAL_SIGNAL_SOFT );
	ASSERT( status == 0 );
	status = sigaddset( &TimerSet, HAL_SIGNAL_CRIT );
	ASSERT( status == 0 );
	status = sigaddset( &TimerSet, HAL_SIGNAL_TRAMPOLINE );
	ASSERT( status == 0 );

	//Create the SoftSet
	status = sigemptyset( &SoftSet );
	ASSERT( status == 0 );
	status = sigaddset( &SoftSet, HAL_SIGNAL_SOFT );
	ASSERT( status == 0 );
	status = sigaddset( &SoftSet, HAL_SIGNAL_CRIT );
	ASSERT( status == 0 );
	status = sigaddset( &SoftSet, HAL_SIGNAL_TRAMPOLINE );
	ASSERT( status == 0 );

	//Create the CritSet
	status = sigemptyset( &CritSet );
	ASSERT( status == 0 );
	status = sigaddset( &CritSet, HAL_SIGNAL_CRIT );
	ASSERT( status == 0 );
	status = sigaddset( &CritSet, HAL_SIGNAL_TRAMPOLINE );
	ASSERT( status == 0 );

	//Create the trampoline set
	status = sigemptyset( &TrampolineSet );
	ASSERT( status == 0 );
	status = sigaddset( &TrampolineSet, HAL_SIGNAL_TRAMPOLINE );
	ASSERT( status == 0 );

	//Create the timer action.
	TimerAction.sa_handler = HalUnixTimer;
	TimerAction.sa_mask = TimerSet;
	status = sigemptyset( &TimerAction.sa_mask);
	ASSERT( status ==  0 );
	TimerAction.sa_flags = 0;

	//Create the soft action.
	SoftAction.sa_handler = SoftHandler;
	SoftAction.sa_mask = SoftSet;
	status = sigemptyset( &SoftAction.sa_mask);
	ASSERT( status ==  0 );
	SoftAction.sa_flags = 0;

	//Create the crit action.
	CritAction.sa_handler = CritHandler;
	CritAction.sa_mask = CritSet;
	status = sigemptyset( &CritAction.sa_mask);
	ASSERT( status ==  0 );
	CritAction.sa_flags = 0;

	//Create the SwitchStackAction 
	SwitchStackAction.sa_handler = HalStackTrampoline;
	SwitchStackAction.sa_mask = TrampolineSet;
	status = sigemptyset( &SwitchStackAction.sa_mask );
	ASSERT( status == 0 );
	SwitchStackAction.sa_flags = SA_ONSTACK;
	status = sigaction(HAL_SIGNAL_TRAMPOLINE, &SwitchStackAction, NULL );

	//The current set should be equal to the timer set.
	status = sigprocmask( SIG_SETMASK, &TimerSet, NULL );
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

	//Turn on the timer signal handler.
	status = sigaction( HAL_SIGNAL_TIMER, &TimerAction, NULL );
	ASSERT( status == 0 );	

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

	status = sigprocmask( SIG_BLOCK, &TimerSet, NULL ); 
	ASSERT( status == 0 );

	ASSERT( HalIsAtomic() );
}

void HalEnableInterrupts()
{
	int status;

	status = sigprocmask( SIG_UNBLOCK, &TimerSet, NULL );
	ASSERT( status == 0 );

	ASSERT( ! HalIsAtomic() );
}

void HalDisableSoftInterrupts()
{
	int status;

	status = sigprocmask( SIG_BLOCK, &SoftSet, NULL ); 
	ASSERT( status == 0 );
}

void HalEnableSoftInterrupts()
{
	int status;

	status = sigprocmask( SIG_UNBLOCK, &SoftSet, NULL );
	ASSERT( status == 0 );
}

void HalDisableCritInterrupts()
{
	int status;

	status = sigprocmask( SIG_BLOCK, &CritSet, NULL ); 
	ASSERT( status == 0 );
}

void HalEnableCritInterrupts()
{
	int status;

	status = sigprocmask( SIG_UNBLOCK, &CritSet, NULL );
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

/*
 * Handler for the Soft ISRs. 
 * This is just a stub.
 */
void SoftHandler( int SignalNumber )
{
	abort(); 
}

/*
 * Handler for the Crit ISRs. 
 * This is just a stub.
 */
void CritHandler( int SignalNumber )
{
	abort(); 
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


		status = sigprocmask( SIG_UNBLOCK, &TrampolineSet, NULL );
		ASSERT( status == 0 );

		raise( HAL_SIGNAL_TRAMPOLINE );

		while( ! halTempContextProcessed );

		status = sigprocmask(SIG_BLOCK, &TrampolineSet, NULL );
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

