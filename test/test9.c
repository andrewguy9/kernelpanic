#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"
#include"kernel/watchdog.h"
#include"kernel/sleep.h"

/*
 * Tests the watchdog.
 * Starts three threads each with different quantums.
 * Each thread increments a variable, Value1 2 and 3.
 * Value1 = 2 * Value2
 * Value2 = 2 * Value3.
 */

//Note: Because Unix does not provide good real time support,
//using small quantums can lead to delayed timer delivery.
//This will cause problems when used with the watchdog.
#define THREAD1QUANTUM 30
#define THREAD2QUANTUM 60
#define THREAD3QUANTUM 90

#define TIMEOUT 2*(THREAD1QUANTUM+THREAD2QUANTUM+THREAD3QUANTUM)

struct THREAD_CONTEXT
{
        COUNT Value;
        INDEX WatchdogId;
};

//
//Main routine for threads.
//

THREAD_MAIN TestThreadMain;
void * TestThreadMain( void * arg )
{
        struct THREAD_CONTEXT * context = (struct THREAD_CONTEXT *) arg;

        WatchdogAddFlag(context->WatchdogId);

        while( 1 )
        {
                context->Value++;
                WatchdogNotify(context->WatchdogId);
        }
        return NULL;
}

//
//Vars 
//

struct THREAD_CONTEXT Value1 = {0, 0};
struct THREAD_CONTEXT Value2 = {0, 1};
struct THREAD_CONTEXT Value3 = {0, 2};

//
//Thread structures
//

#define STACK_SIZE HAL_MIN_STACK_SIZE

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

        SchedulerStartup();

        //Initialize Threads
        SchedulerCreateThread(
                        &TestThreadIncrement,
                        THREAD1QUANTUM,
                        TestThreadStackIncrement,
                        STACK_SIZE,
                        TestThreadMain,
                        &Value1,
                        NULL,
                        true);

        SchedulerCreateThread(
                        &TestThreadDivide,
                        THREAD2QUANTUM,
                        TestThreadStackDivide,
                        STACK_SIZE,
                        TestThreadMain,
                        &Value2,
                        NULL,
                        true);

        SchedulerCreateThread(
                        &TestThreadExp,
                        THREAD3QUANTUM,
                        TestThreadStackExp,
                        STACK_SIZE,
                        TestThreadMain,
                        &Value3,
                        NULL,
                        true);

        //Enable the watchdog
        WatchdogEnable( TIMEOUT );

        //Start the kernel.
        KernelStart();
        return 0;
}
