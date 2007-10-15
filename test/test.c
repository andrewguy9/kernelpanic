#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"
#include"../kernel/timer.h"

//Tests of the Timer subsystem.
struct TIMER FrequentTimer;
COUNT FrequentCount;
void FrequentHandler( )
{
	FrequentCount++;
	TimerRegister( 
			&FrequentTimer,
			2,
			FrequentHandler );
}

struct TIMER SeldomTimer;
COUNT SeldomCount;
void SeldomHandler( )
{
	SeldomCount++;
	TimerRegister(
			&SeldomTimer,
			3,
			SeldomHandler );
}

//Tests of the multithreading system
struct SEMAPHORE ValueLock;
unsigned int Value = 0;

struct THREAD TestThreadIncrement;
char TestThreadStackIncrement[500];
void TestMainIncrement()
{
	unsigned long int a = 0;
	while( 1 )
	{
		//SemaphoreLock( & ValueLock );
		for( a=0; a<0xffffffff; a++);
		Value++;
		for( a=0; a<0xffffffff; a++);
		//SemaphoreUnlock( & ValueLock );

	}
}

struct THREAD TestThreadDivide;
char TestThreadStackDivide[500];
void TestMainDivide()
{
	unsigned long int a = 0;
	while( 1 )
	{
		//SemaphoreLock( & ValueLock );
		for( a=0; a<0xffffffff; a++);
		Value/2;
		for( a=0; a<0xffffffff; a++);
		//SemaphoreUnlock( & ValueLock );
	}
}




int main()
{
	//Initialize the kernel structures.
	KernelInit();

	//Initialize timers.
	FrequentCount = 0;
	TimerRegister( 
			&FrequentTimer,
			2,
			FrequentHandler );
	SeldomCount = 0;
	TimerRegister( 
			&SeldomTimer,
			3,
			SeldomHandler );
			

	//Initialize Threads
	SemaphoreInit( &ValueLock, 1 );
	SchedulerCreateThread(
			&TestThreadIncrement,
			2,
			TestThreadStackIncrement,
			500,
			TestMainIncrement );
	SchedulerCreateThread(
			&TestThreadDivide,
			10,
			TestThreadStackDivide,
			500,
			TestMainDivide);

	KernelStart();
	return 0;
}
