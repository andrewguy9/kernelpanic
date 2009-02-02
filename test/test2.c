#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/socket.h"
#include"../kernel/panic.h"

/*
 * Tests the socket unit, and by extension the resource and ringbuffer units.
 * Overtime TotalRead = TotalWrite +- RingSize.
 */

//
//Tests the Producer consumer model for the socket system. 
//

#define MESSAGE_LENGTH 20
char Message[MESSAGE_LENGTH] = "Thread text message";

//Allocation for buffers.
#define RING_SIZE 64
char RingBuff[RING_SIZE];

struct PIPE Pipe;

struct SOCKET Socket;

//Allocation for workers. 

#ifdef PC_BUILD 
#define STACK_SIZE 0x5000
#endif

#ifdef AVR_BUILD
#define STACK_SIZE 0x500
#endif

char ProducerStack1[STACK_SIZE];
char ProducerStack2[STACK_SIZE];

char ConsumerStack1[STACK_SIZE];
char ConsumerStack2[STACK_SIZE];
char ConsumerStack3[STACK_SIZE];

struct THREAD Producer1;
struct THREAD Producer2;

struct THREAD Consumer1;
struct THREAD Consumer2;
struct THREAD Consumer3;

//Tracking variables
COUNT TotalRead;
COUNT TotalWrite;

//Functions for test.
void ProducerMain()
{
	while(1)
	{
		SocketWriteStruct( Message, MESSAGE_LENGTH, &Socket );

		SchedulerStartCritical();
		TotalWrite += MESSAGE_LENGTH;
		SchedulerEndCritical();
	}
}

void ConsumerMain()
{
	char buff[MESSAGE_LENGTH];

	COUNT index;
	while(1)
	{
		SocketReadStruct( buff, MESSAGE_LENGTH, &Socket );
		
		for( index = 0; index < MESSAGE_LENGTH; index++ )
		{
			if( Message[index] != buff[index] )
				KernelPanic( );
		}

		SchedulerStartCritical();
		TotalRead+=MESSAGE_LENGTH;
		SchedulerEndCritical();
	}
}

//main
int main()
{
	KernelInit();

	//Initialize counters
	TotalRead = 0;
	TotalWrite = 0;

	//Initialize Pipes.
	PipeInit( RingBuff, RING_SIZE, &Pipe );

	//Initialize Socket
	SocketInit( & Pipe, & Pipe, & Socket ); 

	//Initialize Threads
	SchedulerCreateThread(
			&Producer1,
			1,
			ProducerStack1,
			STACK_SIZE,
			ProducerMain,
			NULL,
			0,
			TRUE);
	SchedulerCreateThread(
			&Producer2,
			1,
			ProducerStack2,
			STACK_SIZE,
			ProducerMain,
			NULL,
			1,
			TRUE);
	SchedulerCreateThread(
			&Consumer1,
			1,
			ConsumerStack1,
			STACK_SIZE,
			ConsumerMain,
			NULL,
			2,
			TRUE);
	SchedulerCreateThread(
			&Consumer2,
			1,
			ConsumerStack2,
			STACK_SIZE,
			ConsumerMain,
			NULL,
			4,
			TRUE);

	SchedulerCreateThread(
			&Consumer3,
			1,
			ConsumerStack3,
			STACK_SIZE,
			ConsumerMain,
			NULL,
			5,
			TRUE);
	//Kick off the kernel.
	KernelStart();
	return 0;
}
