#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/hal.h"
#include"../kernel/serial.h"

/*
 * Starts three threads each with different quantums.
 * Each thread increments a variable, Value1 2 and 3.
 * Value1 = 2 * Value2 
 * Value2 = 2 * Value3.
 */

//
//Main routine for threads.
//

#define BUF_SIZE 512

void Test1ThreadMain( void * arg )
{
	COUNT * var = (COUNT *) arg;
	char buf[BUF_SIZE];
	COUNT read;

	volatile unsigned char a = 0;
	while( 1 )
	{
		(*var)++;
		for(a=1;a>0;a++);

		read = SerialRead(buf, BUF_SIZE);
		buf[read]= '\0';
		if(read > 0) {
			SerialWrite("Read: ", 6);
			SerialWrite(buf, read);
			SerialWrite("\n", 1);
		}
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

	SchedulerStartup();
	SerialStartup();
	
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
