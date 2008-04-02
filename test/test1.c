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
//Tests of the multithreading system
//
volatile unsigned int Value1 = 0;
volatile unsigned int Value2 = 0;
volatile unsigned int Value3 = 0;
struct THREAD TestThreadIncrement;
char TestThreadStackIncrement[500];
void TestMainIncrement()
{
	volatile unsigned char a = 0;
	HalToggleDebugLedFlag(3);
	while( 1 )
	{
		Value1++;
		HalToggleDebugLedFlag(1);
		for(a=1;a>0;a++);

	}
}

struct THREAD TestThreadDivide;
char TestThreadStackDivide[500];
void TestMainDivide()
{
	volatile unsigned char a = 0;
	while( 1 )
	{
		Value2++;
		HalToggleDebugLedFlag(2);
		for(a=1;a>0;a++);
	}
}

struct THREAD TestThreadExp;
char TestThreadStackExp[500];
void TestMainExp()
{
	volatile unsigned char a = 0;
	while( 1 )
	{
		Value3++;
		HalToggleDebugLedFlag(3);
		for(a=1;a>0;a++);
	}
}



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
			TestMainIncrement,
		   	4,
			TRUE);

	SchedulerCreateThread(
			&TestThreadDivide,
			4,
			TestThreadStackDivide,
			500,
			TestMainDivide,
			5,
			TRUE);

	SchedulerCreateThread(
			&TestThreadExp,
			8,
			TestThreadStackExp,
			500,
			TestMainExp,
			6,
			TRUE);
		
	KernelStart();
	return 0;
}
