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

#ifdef PC_BUILD 
#define STACK_SIZE 0x5000
#endif

#ifdef AVR_BUILD
#define STACK_SIZE 0x500
#endif

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
			2,
			TestThreadStackIncrement,
			500,
			Test1ThreadMain,
			&Value1,
		   	4,
			TRUE);

	SchedulerCreateThread(
			&TestThreadDivide,
			4,
			TestThreadStackDivide,
			500,
			Test1ThreadMain,
			&Value2,
			5,
			TRUE);

	/*
	SchedulerCreateThread(
			&TestThreadExp,
			8,
			TestThreadStackExp,
			500,
			Test1ThreadMain,
			&Value3,
			6,
			TRUE);
*/		
	KernelStart();
	return 0;
}
