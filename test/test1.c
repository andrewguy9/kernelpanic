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
	COUNT * var = arg;
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

struct THREAD TestThreadIncrement;
char TestThreadStackIncrement[500];

struct THREAD TestThreadDivide;
char TestThreadStackDivide[500];

struct THREAD TestThreadExp;
char TestThreadStackExp[500];

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
			2,
			TestThreadStackIncrement,
			500,
			Test1ThreadMain,
			&Value1,
		   	0x10,
			TRUE);

	SchedulerCreateThread(
			&TestThreadDivide,
			4,
			TestThreadStackDivide,
			500,
			Test1ThreadMain,
			&Value2,
			0X20,
			TRUE);

	SchedulerCreateThread(
			&TestThreadExp,
			8,
			TestThreadStackExp,
			500,
			Test1ThreadMain,
			&Value3,
			0X40,
			TRUE);
		
	KernelStart();
	return 0;
}
