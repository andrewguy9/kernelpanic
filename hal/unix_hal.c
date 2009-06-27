//
//PC CODE
//

//
//Common code between all flavors of unix
//

#include<sys/time.h>
#include<string.h>
#include<signal.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

#define AlarmSignal SIGALRM

char HAL_WATCHDOG_MASK;
BOOL HalWatchdogOn;
unsigned int HalWatchDogFrequency;
unsigned int HalWatchdogCount;

struct itimerval TimerInterval;

//Used for reference.
sigset_t EmptySet;
sigset_t TimerSet;

//Storage for the signal mask.
sigset_t CurrentSet;

//Timer Action Storage.
struct sigaction TimerAction;

//Prototype for later use.
void HalUnixTimer( int SignalNumber );

void HalStartup()
{
	int status;

	//Create the empty set.
	status = sigemptyset( &EmptySet );
	ASSERT( status == 0 );

	//Create the timer set.
	status = sigemptyset( &TimerSet );
	ASSERT( status == 0 );
	status = sigaddset( &TimerSet, AlarmSignal );
	ASSERT( status == 0 );
	ASSERT( sigismember( &TimerSet, AlarmSignal ) == 1 );

	//Create the timer action.
	TimerAction.sa_handler = HalUnixTimer;
	status = sigemptyset( &TimerAction.sa_mask);
	ASSERT( status ==  0 );
	TimerAction.sa_flags = 0;

	//The current set should be equal to the timer set.
	CurrentSet = TimerSet;
	status = sigprocmask( SIG_SETMASK, &CurrentSet, NULL );
	ASSERT( status == 0 );

	ASSERT( HalIsAtomic() );

	//Set up the watchdog.
	HalWatchdogOn = FALSE;
	HAL_WATCHDOG_MASK = 0;
	HalWatchDogFrequency = 0;
	HalWatchdogCount = 0;
}

void HalInitClock()
{
	int status;

	//Turn on the timer signal handler.
	status = sigaction( AlarmSignal, &TimerAction, NULL );
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

	status = sigismember( &curSet, AlarmSignal );
	ASSERT( status != -1 );
	if( status == 1 )
	{
		//Alarm was a member of the "blocked" mask, so we are atomic.
		return TRUE;
	}
	else 
	{
		//Alarm was not a member of the "blocked" mask, so we are not atomic.
		return FALSE; 
	}
}
#endif //DEBUG

void HalDisableInterrupts()
{
	int status;

	status = sigprocmask( SIG_SETMASK, &TimerSet, NULL ); 
	ASSERT( status == 0 );

	//We just disabled,
	//update current set.
	CurrentSet = TimerSet;

	ASSERT( HalIsAtomic() );
}

void HalEnableInterrupts()
{
	int status;

	//We are about to enable, update current set.
	CurrentSet = EmptySet;

	status = sigprocmask( SIG_SETMASK, &EmptySet, NULL );
	ASSERT( status == 0 );

	ASSERT( ! HalIsAtomic() );
}

void HalPanic(char file[], int line)
{
	printf("PANIC: %s:%d\n",file,line);
	exit(-1);
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
 * Calls TimerInterrupt if he can.
 */
void HalUnixTimer( int SignalNumber )
{
	//The kernel should add this signal to the blocked list inorder to avoid 
	//nesting calls the the handler.
	//verify this.
	ASSERT( HalIsAtomic() );

	//The current mask changed when this was called. 
	//save the change over the mask.
	CurrentSet = TimerSet;

	//TODO IF POSSIBLE, MOVE WATCHDOG INTO OWN TIMER.
	//Run the watchdog check
	if( HalWatchdogOn )
	{
		HalWatchdogCount ++;

		if( HalWatchdogCount >= HalWatchDogFrequency )
		{
			//The time for a match has expried. Panic!!!
			HalPanic("Watchdog Timeout", 0 );
		}
	}

	//Call the kernel's timer handler.
	TimerInterrupt();
}

//
//Darwin only code
//

#ifdef DARWIN 

#define ESP_OFFSET 9
#define EBP_OFFSET 8

void HalCreateStackFrame( 
		struct MACHINE_CONTEXT * Context, 
		void * stack, 
		STACK_INIT_ROUTINE foo, 
		COUNT stackSize)
{

	int status;
	unsigned char * top;
	unsigned int *esp;
	unsigned int *ebp;
	unsigned char * cstack = stack;

	//sigset_t oldSet;

	ASSERT( HalIsAtomic() );

	status = _setjmp( Context->Registers );

	if( status == 0 )
	{
		//Because status was 0 we know that this is the creation of
		//the stack frame. We can use the locals to construct the frame.
	
		//We need to store foo into the machine context so we know who to call
		//when the new frame is activated.
		Context->Foo = foo;

		//Calculate the top of the stack
		//For some kernels we need the stack to be 16 byte alligned.
		top = &cstack[stackSize];
		top = top - sizeof( sigjmp_buf );
		top = (unsigned char *)(((unsigned int) top) & 0xFFFFFFF0);
		ASSERT( ((unsigned int) top) % 16 == 0 );
		ASSERT( (unsigned int) top > (unsigned int) stack );


		//We need to write new stack pointer into the register buffer.
		esp = (unsigned int*) ( ((unsigned char *) &Context->Registers)+( ESP_OFFSET*sizeof(int)) );
		*esp = (int) top;
		//We need to write new base pointer into the register buffer.
		ebp = (unsigned int*) ( ((unsigned char *) &Context->Registers)+( EBP_OFFSET*sizeof(int)) );
		*ebp = (int) top;

#ifdef DEBUG
		//Set up the stack boundry.
		Context->High = (char *) top;
		Context->Low = stack;
#endif

		return;
	}
	else
	{
		//If we get here, then someone has jumped into a newly created thread.
		//Test to make sure we are atomic
		ASSERT( HalIsAtomic() );

		//On linux systems we call foo directly because those 
		//fuckers hide their program registers somwhere.

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

#endif  //end DARWIN

//
//Linux and BSD Code
//

#if LINUX || BSD

#include<ucontext.h>

void HalCreateStackFrame( 
		struct MACHINE_CONTEXT * Context, 
		void * stack, 
		STACK_INIT_ROUTINE foo, 
		COUNT stackSize)
{
	(void)getcontext( &Context->uc );

	/* adjust to new context */
	Context->uc.uc_link = NULL;
	Context->uc.uc_stack.ss_sp = stack;
	Context->uc.uc_stack.ss_size = stackSize;
	Context->uc.uc_stack.ss_flags = 0;

	/*make new context */
	makecontext( &(Context->uc), foo, 0 );

#ifdef DEBUG
	//Set up the stack boundry.
	Context->High = stack + stackSize;
	Context->Low = stack;
#endif
}

void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context )
{
	/* printf("hal creating idle loop frame\n"); */
	(void)getcontext( &Context->uc);

#ifdef DEBUG
	//The stack bounderies are infinite for the initial stack.
	Context->High = (char *) -1;
	Context->Low = (char *) 0;
#endif
}

void HalContextSwitch( )
{
	struct MACHINE_CONTEXT * oldContext = ActiveStack;
	struct MACHINE_CONTEXT * newContext = NextStack;

	ActiveStack = NextStack;
	NextStack = NULL;

	(void)swapcontext(&oldContext->uc, &newContext->uc);
}

#endif //end of LINUX || BSD

