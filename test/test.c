#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"

struct THREAD TestThreadIncrement;
char TestThreadStackIncrement[500];

struct THREAD TestThreadDivide;
char TestThreadStackDivide[500];

struct SEMAPHORE ValueLock;
unsigned int Value = 0;

void TestMainIncrement()
{
	int a = 0;
	while( 1 )
	{
		SemaphoreLock( & ValueLock );
		for( a=0; a<0xffff; a++);
		Value++;
		for( a=0; a<0xffff; a++);
		SemaphoreUnlock( & ValueLock );

	}
}

void TestMainDivide()
{
	int a = 0;
	while( 1 )
	{
		SemaphoreLock( & ValueLock );
		for( a=0; a<0xffff; a++);
		Value/2;
		for( a=0; a<0xffff; a++);
		SemaphoreUnlock( & ValueLock );
	}
}

int main()
{
	KernelInit();
	SemaphoreInit( &ValueLock, 1 );
	SchedulerCreateThread(
			&TestThreadIncrement,
			2,
			TestThreadStackIncrement,
			500,
			TestMainIncrement );
	SchedulerCreateThread(
			&TestThreadDivide,
			10,
			TestThreadStackDivide,
			500,
			TestMainDivide);
	KernelStart();
	return 0;
}
