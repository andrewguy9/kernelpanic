#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"
#include"../kernel/timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#define DEBUG_LED          PORTC
#define DEBUG_LED_DDR      DDRC

//Tests of the Timer subsystem.
struct TIMER FrequentTimer;
COUNT FrequentCount;
void FrequentHandler( void * arg )
{
	FrequentCount++;
	TimerRegister( 
			&FrequentTimer,
			2,
			FrequentHandler,
		   NULL	);
}

struct TIMER SeldomTimer;
COUNT SeldomCount;
void SeldomHandler( void * arg )
{
	SeldomCount++;
	TimerRegister(
			&SeldomTimer,
			3,
			SeldomHandler,
		   NULL	);
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
	volatile unsigned int a = 0;
	DEBUG_LED ^= 1<<3;
	while( 1 )
	{
		//SemaphoreLock( & ValueLock );
		Value1++;
		DEBUG_LED ^= 1<<0;
		for(a=1;a>0;a++);
		SchedulerStartCritical();
		SchedulerForceSwitch();
		//SemaphoreUnlock( & ValueLock );

	}
}

struct THREAD TestThreadDivide;
char TestThreadStackDivide[500];
void TestMainDivide()
{
	volatile unsigned int a = 0;
	while( 1 )
	{
		//SemaphoreLock( & ValueLock );
		Value2++;
		DEBUG_LED ^= 1<<1;
		for(a=1;a>0;a++);
		SchedulerStartCritical();
		SchedulerForceSwitch();
		//SemaphoreUnlock( & ValueLock );
	}
}

struct THREAD TestThreadExp;
char TestThreadStackExp[500];
void TestMainExp()
{
	volatile unsigned int a = 0;
	while( 1 )
	{
		//SemaphoreLock( & ValueLock );
		Value3++;
		DEBUG_LED ^= 1<<2;
		for(a=1;a>0;a++);
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
	FrequentCount = 0;
	TimerRegister( 
			&FrequentTimer,
			2,
			FrequentHandler,
		   NULL	);
	SeldomCount = 0;
	TimerRegister( 
			&SeldomTimer,
			3,
			SeldomHandler,
		   NULL	);

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
