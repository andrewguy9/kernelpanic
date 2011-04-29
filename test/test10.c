#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/hal.h"
#include"../kernel/serial.h"

/*
 * Reads from STDIN and prints the results to STDOUT. 
 */

//
//Main routine for threads.
//

#define BUF_SIZE 512

void TestThreadMain( void * arg )
{
	char buf[BUF_SIZE];
	COUNT read;

	while( 1 )
	{
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
		   	4,
			TRUE);

	KernelStart();
	return 0;
}
