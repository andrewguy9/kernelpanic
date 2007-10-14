#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"

struct THREAD TestThreadIncrement;
char TestThreadStackIncrement[500];

struct THREAD TestThreadDivide;
char TestThreadStackDivide[500];

SEMAPHORE ValueLock;
unsigned int Value = 0;

void TestMainIncrament()
{
	int a = 0;
	for( a=0; a<10000; a++)
	{
		SemaphoreLock( & ValueLock );
		Value++;
		SemaphoreUnlock( & ValueLock );
	}
}

void TestMainDivide()
{
	int a = 0;
	for( a=0; a<10000; a++)
	{
		SemaphoreLock( &ValueLock );
		Value/=2;
		Semaphoreunlock( &ValueLock );
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
			TestMain );
	ScheduerCreateThread(
			&TestThreadDivide,
			10,
			TestThreadStackDivide,
			500,
			TestMain);
	KernelStart();
	return 0;
}
