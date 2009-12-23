#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/hal.h"
#include"../kernel/watchdog.h"

/*
 * Starts three threads each with different quantums.
 * Each thread increments a variable, Value1 2 and 3.
 * Value1 = 2 * Value2 
 * Value2 = 2 * Value3.
 */

#define THREAD1FLAG 1
#define THREAD2FLAG 2
#define THREAD3FLAG 3

#define THREAD1QUANTUM 1
#define THREAD2QUANTUM 2
#define THREAD3QUANTUM 3

#define FREQ 2*(THREAD1QUANTUM+THREAD2QUANTUM+THREAD3QUANTUM)

//
//Main routine for threads.
//

void TestThreadMain( void * arg )
{
	COUNT * var = (COUNT *) arg;
	volatile unsigned char a = 0;
	while( 1 )
	{
		(*var)++;
		for(a=1;a>0;a++);

	}
}

//
//Vars 
//

COUNT Value1 = 0;
COUNT Value2 = 0;
COUNT Value3 = 0;

//
//Thread structures
//

#define STACK_SIZE HAL_MIN_STACK_SIZE

struct THREAD TestThreadIncrement;
char TestThreadStackIncrement[STACK_SIZE];

struct THREAD TestThreadDivide;
char TestThreadStackDivide[STACK_SIZE];

struct THREAD TestThreadExp;
char TestThreadStackExp[STACK_SIZE];

//
//Main
//

int main()
{
	//Initialize the kernel structures.
	KernelInit();

	//Initialize Threads
	SchedulerCreateThread(
			&TestThreadIncrement,
			THREAD1QUANTUM,
			TestThreadStackIncrement,
			STACK_SIZE,
			TestThreadMain,
			&Value1,
		   	THREAD1FLAG,
			TRUE);

	SchedulerCreateThread(
			&TestThreadDivide,
			THREAD2QUANTUM,
			TestThreadStackDivide,
			STACK_SIZE,
			TestThreadMain,
			&Value2,
			THREAD2FLAG,
			TRUE);

	SchedulerCreateThread(
			&TestThreadExp,
			THREAD3QUANTUM,
			TestThreadStackExp,
			STACK_SIZE,
			TestThreadMain,
			&Value3,
			THREAD3FLAG,
			TRUE);

	//Enable the watchdog
	WatchdogEnable( FREQ );

	//Start the kernel.
	KernelStart();
	return 0;
}
