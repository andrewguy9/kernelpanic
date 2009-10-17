#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/hal.h"

/*
 * Starts three threads each with different quantums.
 * Each thread increments a variable, Value1 2 and 3.
 * Value1 = 2 * Value2 
 * Value2 = 2 * Value3.
 */

//
//Main routine for threads.
//

void Test1ThreadMain( void * arg )
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

struct THREAD TestThread1;
char TestThread1Stack[STACK_SIZE];

struct THREAD TestThread2;
char TestThread2Stack[STACK_SIZE];

struct THREAD TestThread3;
char TestThread3Stack[STACK_SIZE];

//
//Main
//

int main()
{
	//Initialize the kernel structures.
	KernelInit();

	//Initialize Threads
	SchedulerCreateThread(
			&TestThread1,
			2,
			TestThread1Stack,
			STACK_SIZE,
			Test1ThreadMain,
			&Value1,
		   	4,
			TRUE);

	SchedulerCreateThread(
			&TestThread2,
			4,
			TestThread2Stack,
			STACK_SIZE,
			Test1ThreadMain,
			&Value2,
			5,
			TRUE);

	SchedulerCreateThread(
			&TestThread3,
			8,
			TestThread3Stack,
			STACK_SIZE,
			Test1ThreadMain,
			&Value3,
			6,
			TRUE);

	KernelStart();
	return 0;
}
