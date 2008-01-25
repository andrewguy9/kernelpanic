#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/hal.h"

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
	DEBUG_LED ^= 1<<3;
	while( 1 )
	{
		Value1++;
		DEBUG_LED ^= 1<<0;
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
		DEBUG_LED ^= 1<<1;
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
		DEBUG_LED ^= 1<<2;
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
		   	0x10,
			TRUE);

	SchedulerCreateThread(
			&TestThreadDivide,
			4,
			TestThreadStackDivide,
			500,
			TestMainDivide,
			0X20,
			TRUE);

	SchedulerCreateThread(
			&TestThreadExp,
			8,
			TestThreadStackExp,
			500,
			TestMainExp,
			0X40,
			TRUE);
		
	KernelStart();
	return 0;
}
