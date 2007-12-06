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
char buff1[RING_SIZE];
char buff2[RING_SIZE];

struct PIPE Pipe1;
struct PIPE Pipe2;

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

//Functions for test.
void ProducerMain()
{
	COUNT write;
	COUNT count = 0;
	while(1)
	{
		write = SocketWrite( Message, MESSAGE_LENGTH, &Socket );

		if( write != MESSAGE_LENGTH )
			KernelPanic( 0 );
		count++;
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
			KernelPanic( 0 );
		for( index = 0; index < read; index++ )
		{
			if( Message[index] != buff[index] )
				KernelPanic(0);
		}
	}
}

//main
int main()
{
	//Initialize Pipes.
	PipeInit( buff1, RING_SIZE, &Pipe1 );
	PipeInit( buff2, RING_SIZE, &Pipe2 );

	//Initialize Threads
	SchedulerCreateThread(
			&Producer1,
			1,
			ProducerStack1,
			STACK_SIZE,
			ProducerMain);

	SchedulerCreateThread(
			&Producer2,
			1,
			ProducerStack2,
			STACK_SIZE,
			ProducerMain);

	SchedulerCreateThread(
			&Consumer1,
			1,
			ConsumerStack1,
			STACK_SIZE,
			ConsumerMain);

	SchedulerCreateThread(
			&Consumer2,
			1,
			ConsumerStack2,
			STACK_SIZE,
			ConsumerMain);

	SchedulerCreateThread(
			&Consumer3,
			1,
			ConsumerStack3,
			STACK_SIZE,
			ConsumerMain);

	//Kick off the kernel.
	KernelStart();
	return 0;
}
