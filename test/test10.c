#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/hal.h"
#include"../kernel/interrupt.h"

#include<stdio.h>

/*
 * Starts three threads each with different quantums.
 * Each thread increments a variable, Value1 2 and 3.
 * Value1 = 2 * Value2 
 * Value2 = 2 * Value3.
 */

//
//Processing routines.
//

void Break()
{
}

int RunningSum( int a, int b, int dist )
{
	if( dist == 0 )
		return a+b;
	else
	{
		return a+b+RunningSum(a,b,dist-1);
	}
}

int RunningProduct( int a )
{
	if( a == 1 )
		return 1;
	else
		return a*RunningProduct(a-1);
}

//
//Main routine for threads.
//

void RunningSumMain( void * arg )
{
	int result = -1;
	while(TRUE)
	{

		InterruptDisable();

		Break();

		InterruptDisable();
		printf("start sum\n");
		InterruptEnable();
		
		InterruptDisable();
		result = RunningSum(1,2,3);
		ASSERT( result == 12 );
		InterruptEnable();

		InterruptDisable();
		printf("sum %d\n", result);
		InterruptEnable();

		InterruptEnable();
	}
}

void RunningProductMain( void * arg )
{
	int result;
	while(TRUE)
	{
		InterruptDisable();

		InterruptDisable();
		printf("start product\n");
		InterruptEnable();
		
		InterruptDisable();
		result = RunningProduct(5);
		InterruptEnable();

		InterruptDisable();
		printf("product %d\n", result);
		InterruptEnable();

		InterruptEnable();
	}
}

//
//Thread structures
//

#ifdef PC_BUILD 
#define STACK_SIZE (0x5000*20)
#endif

#ifdef AVR_BUILD
#define STACK_SIZE 0x500
#endif

struct THREAD RunningSumThread;
char RunningSumStack[STACK_SIZE];

struct THREAD RunningProductThread;
char RunningProductStack[STACK_SIZE];

//
//Main
//

int main()
{
	printf("start\n");
	//Initialize the kernel structures.
	KernelInit();

	//Initialize Threads
	SchedulerCreateThread(
			&RunningSumThread,
			2,
			RunningSumStack,
			STACK_SIZE,
			RunningSumMain,
			NULL,
		   	4,
			TRUE);

	SchedulerCreateThread(
			&RunningProductThread,
			4,
			RunningProductStack,
			STACK_SIZE,
			RunningProductMain,
			NULL,
			5,
			TRUE);

	printf("kicing off\n");

	KernelStart();
	return 0;
}
