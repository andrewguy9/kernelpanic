#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/socket.h"
#include"../kernel/panic.h"

//
//Tests the Producer consumer model for the socket system. 
//

char Message[] = "Thread text message";
#define MESSAGE_LENGTH 20

//Allocation for buffers.
#define RING_SIZE 64
char RingBuff[RING_SIZE];

struct PIPE Pipe;

struct SOCKET Socket;

//Allocation for workers. 
#define STACK_SIZE 300
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
	COUNT write;
	while(1)
	{
		write = SocketWrite( Message, MESSAGE_LENGTH, &Socket );

		if( write != MESSAGE_LENGTH )
			KernelPanic( TEST2_PRODUCER_WRITE_WRONG_LENGTH );

		SchedulerStartCritical();
		TotalWrite += write;
		SchedulerEndCritical();
	}
}

void ConsumerMain()
{
	char buff[MESSAGE_LENGTH];

	COUNT read;
	COUNT index;
	while(1)
	{
		read = SocketReadStruct( buff, MESSAGE_LENGTH, &Socket );
		if( read != MESSAGE_LENGTH )
			KernelPanic( TEST2_CONSUMER_READ_WRONG_LENGTH );
		for( index = 0; index < read; index++ )
		{
			if( Message[index] != buff[index] )
				KernelPanic( TEST2_CONSUMER_CHARACTER_MISMATCH );
		}
		SchedulerStartCritical();
		TotalRead+=read;
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
			0x01,
			TRUE);
	SchedulerCreateThread(
			&Producer2,
			1,
			ProducerStack2,
			STACK_SIZE,
			ProducerMain,
			0x02,
			TRUE);
	SchedulerCreateThread(
			&Consumer1,
			1,
			ConsumerStack1,
			STACK_SIZE,
			ConsumerMain,
			0x10,
			TRUE);
	SchedulerCreateThread(
			&Consumer2,
			1,
			ConsumerStack2,
			STACK_SIZE,
			ConsumerMain,
			0x20,
			TRUE);

	SchedulerCreateThread(
			&Consumer3,
			1,
			ConsumerStack3,
			STACK_SIZE,
			ConsumerMain,
			0x40,
			TRUE);
	//Kick off the kernel.
	KernelStart();
	return 0;
}
