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
volatile unsigned int Value1 = 0;
volatile unsigned int Value2 = 0;
volatile unsigned int Value3 = 0;
struct THREAD TestThreadIncrement;
char TestThreadStackIncrement[500];
void TestMainIncrement()
{
	volatile unsigned long int a = 0;
	while( 1 )
	{
		//SemaphoreLock( & ValueLock );
		Value1++;
		SchedulerStartCritical();
		SchedulerForceSwitch();
		//SemaphoreUnlock( & ValueLock );

	}
}

struct THREAD TestThreadDivide;
char TestThreadStackDivide[500];
void TestMainDivide()
{
	volatile unsigned long int a = 0;
	while( 1 )
	{
		//SemaphoreLock( & ValueLock );
		Value2++;
		SchedulerStartCritical();
		SchedulerForceSwitch();
		//SemaphoreUnlock( & ValueLock );
	}
}

struct THREAD TestThreadExp;
char TestThreadStackExp[500];
void TestMainExp()
{
	volatile unsigned long int a = 0;
	while( 1 )
	{
		//SemaphoreLock( & ValueLock );
		Value3++;
		SchedulerStartCritical();
		SchedulerForceSwitch();
		//SemaphoreUnlock( & ValueLock );
	}
}



int main()
{
	//Initialize the kernel structures.
	KernelInit();

	//Initialize timers.
	/*
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
*/
	//Initialize Threads
	SemaphoreInit( &ValueLock, 1 );
	SchedulerCreateThread(
			&TestThreadIncrement,
			5,
			TestThreadStackIncrement,
			500,
			TestMainIncrement );
	SchedulerCreateThread(
			&TestThreadDivide,
			5,
			TestThreadStackDivide,
			500,
			TestMainDivide);
	SchedulerCreateThread(
			&TestThreadExp,
			5,
			TestThreadStackExp,
			500,
			TestMainExp);
		
	KernelStart();
	return 0;
}
