#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"

/*
 * Starts three threads each with different quantums.
 * Each thread increments a variable, Value1 2 and 3.
 * Value1 = 2 * Value2 
 * Value2 = 2 * Value3.
 */

#define ITERATIONS 1000000

//
//Main routine for threads.
//

THREAD_MAIN Test1ThreadMain;
void * Test1ThreadMain( void * arg )
{
        SchedulerShutdown( );
	COUNT * var = (COUNT *) arg;
	while( 1 )
	{
		(*var)++;
		for(unsigned char a=1;a>0;a++) {}
                if (*var > ITERATIONS) {
                  return NULL;
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

        //Initialize Threads
        SchedulerCreateThread(
                        &TestThread1,
                        2,
                        TestThread1Stack,
                        STACK_SIZE,
                        Test1ThreadMain,
                        &Value1,
                        NULL,
                        true);

        SchedulerCreateThread(
                        &TestThread2,
                        4,
                        TestThread2Stack,
                        STACK_SIZE,
                        Test1ThreadMain,
                        &Value2,
                        NULL,
                        true);

        SchedulerCreateThread(
                        &TestThread3,
                        8,
                        TestThread3Stack,
                        STACK_SIZE,
                        Test1ThreadMain,
                        &Value3,
                        NULL,
                        true);

        KernelStart();
        return 0;
}
