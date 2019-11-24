#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/hal.h"
#include"kernel/serial.h"

/*
 * Reads from STDIN and prints the results to STDOUT.
 */

//
//Main routine for threads.
//

#define BUF_SIZE 512

THREAD_MAIN TestThreadMain;
void * TestThreadMain( void * arg )
{
        char buf[BUF_SIZE];
        COUNT read;

        while( 1 )
        {
                read = SerialRead(buf, BUF_SIZE);
                buf[read]= '\0';
                if(read > 0) {
                        SerialWrite(buf, read);
                }
        }
        return NULL;
}


//
//Thread structures
//

#define STACK_SIZE HAL_MIN_STACK_SIZE

struct THREAD TestThread;
char TestThreadStack[STACK_SIZE];

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
                        &TestThread,
                        2,
                        TestThreadStack,
                        STACK_SIZE,
                        TestThreadMain,
                        NULL,
                        true);

        KernelStart();
        return 0;
}
