#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"

/*
 * Starts three threads each with different quantums.
 * Each thread increments a variable, Value1 2 and 3.
 * Value1 = 2 * Value2 
 * Value2 = 2 * Value3.
 */

//
//Main routine for threads.
//

static THREAD_MAIN Test1ThreadMain;
void Test1ThreadMain( void * arg )
{
	COUNT * var = (COUNT *) arg;
	volatile int a;
	while( TRUE )
	{
		(*var)++;
		for(a= 1;a>0;a++);

	}
}

//
//Vars 
//

static COUNT Value1 = 0;
static COUNT Value2 = 0;
static COUNT Value3 = 0;

//
//Thread structures
//

#define STACK_SIZE HAL_MIN_STACK_SIZE

static struct THREAD TestThread1;
static char TestThread1Stack[STACK_SIZE];

static struct THREAD TestThread2;
static char TestThread2Stack[STACK_SIZE];

static struct THREAD TestThread3;
static char TestThread3Stack[STACK_SIZE];

//
//Main
//

int main()
{
        //Initialize the kernel structures.
        KernelInit();

        SchedulerStartup();

        //Initialize Threads
        SchedulerCreateThread(
                        &TestThread1,
                        2,
                        TestThread1Stack,
                        STACK_SIZE,
                        Test1ThreadMain,
                        &Value1,
                        TRUE);

        SchedulerCreateThread(
                        &TestThread2,
                        4,
                        TestThread2Stack,
                        STACK_SIZE,
                        Test1ThreadMain,
                        &Value2,
                        TRUE);

        SchedulerCreateThread(
                        &TestThread3,
                        8,
                        TestThread3Stack,
                        STACK_SIZE,
                        Test1ThreadMain,
                        &Value3,
                        TRUE);

        KernelStart();
        return 0;
}
