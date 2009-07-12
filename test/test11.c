#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/pipe.h"
#include"../kernel/panic.h"
#include"../kernel/interrupt.h"
#include"../kernel/socket.h"
#include<stdio.h>

/*
 * Tests the pipe unit. 
 */

#define BUFFER_LENGTH 10

char MessageBuffer1[BUFFER_LENGTH];
char MessageBuffer2[BUFFER_LENGTH];
char MessageBuffer3[BUFFER_LENGTH];

#define RANDOM_VALUES_SIZE 15
char RandomNumbers [RANDOM_VALUES_SIZE] = 
{
	0xf, 0x1, 0x2, 
	0x3, 0x0, 0x5, 
	0x6, 0x7, 0x8,
   	0x9, 0xa, 0x0,
	0xc, 0xd, 0xe
};

struct PIPE Pipe1;
struct PIPE Pipe2;
struct PIPE Pipe3;

struct SOCKET Socket1;
struct SOCKET Socket2;
struct SOCKET Socket3;

#ifdef PC_BUILD 
#define STACK_SIZE 0x5000
#endif

#ifdef AVR_BUILD
#define STACK_SIZE 0x500
#endif

char ProducerStack1[STACK_SIZE];
char ProducerStack2[STACK_SIZE];
char ProducerStack3[STACK_SIZE];

char ConsumerStack1[STACK_SIZE];
char ConsumerStack2[STACK_SIZE];
char ConsumerStack3[STACK_SIZE];

struct THREAD Producer1;
struct THREAD Producer2;
struct THREAD Producer3;

struct THREAD Consumer1;
struct THREAD Consumer2;
struct THREAD Consumer3;

volatile COUNT ProducerCount;
volatile COUNT ConsumerCount;

void ProducerMain( void * arg )
{
	struct SOCKET * MySock = ( struct SOCKET *) arg;
	INDEX timeIndex;
	INDEX bufferIndex;
	COUNT length;
	char AssendingBuffer[RANDOM_VALUES_SIZE];
	char DecendingBuffer[RANDOM_VALUES_SIZE];
	BOOL assending;

	char * curBuffer;

	for( bufferIndex = 0; bufferIndex < RANDOM_VALUES_SIZE; bufferIndex++ )
	{
		AssendingBuffer[ bufferIndex ] = bufferIndex;
		DecendingBuffer[ bufferIndex ] = RANDOM_VALUES_SIZE - bufferIndex;
	}

	timeIndex = 0;
	assending = TRUE;

	while(1)
	{
		length = RandomNumbers[timeIndex];

		if( assending )
			curBuffer = AssendingBuffer;
		else
			curBuffer = DecendingBuffer;

		//Perform write
		SocketWriteStruct( 
				curBuffer,
				length,
				MySock );
		
		//Setup next value.
		timeIndex = (timeIndex + 1) % RANDOM_VALUES_SIZE;
		assending = !assending;
	}
}

void ConsumerMain( void * arg )
{
	struct SOCKET * MySock = ( struct SOCKET *) arg;
	INDEX timeIndex;
	COUNT bufferIndex;
	char myBuffer[RANDOM_VALUES_SIZE];
	BOOL assending;
	COUNT length;
   
	timeIndex = 0;
	assending = TRUE;
	
	while(1)
	{
		//Set Buffer up with values which will fail if a bug occurs.
		for( bufferIndex = 0; bufferIndex < RANDOM_VALUES_SIZE; bufferIndex++ )
		{
			if( assending )
				myBuffer[bufferIndex] = 0;
			else
				myBuffer[bufferIndex] = RANDOM_VALUES_SIZE;
		}

		length = RandomNumbers[timeIndex];

		//Perform read
		SocketReadStruct(
				myBuffer,
				length,
				MySock);

		//validate direction of buffer.
		for( bufferIndex = 0; bufferIndex+1 < length; bufferIndex++)
		{
			if( assending )
			{
				if( myBuffer[bufferIndex] >= myBuffer[bufferIndex+1] )
					KernelPanic();
			}
			else
			{
				if( myBuffer[bufferIndex] <= myBuffer[bufferIndex+1] )
					KernelPanic();
			}
		}

		//Setup next value.
		timeIndex = ( timeIndex + 1 ) % RANDOM_VALUES_SIZE;
		assending = !assending;
	}
}


int main()
{
	KernelInit();

	ConsumerCount = 0;
	ProducerCount = 0;

	PipeInit( MessageBuffer1, BUFFER_LENGTH, &Pipe1 );
	PipeInit( MessageBuffer2, BUFFER_LENGTH, &Pipe2 );
	PipeInit( MessageBuffer3, BUFFER_LENGTH, &Pipe3 );

	SocketInit( &Pipe1, &Pipe1, &Socket1 );
	SocketInit( &Pipe2, &Pipe2, &Socket2 );
	SocketInit( &Pipe3, &Pipe3, &Socket3 );

	SchedulerCreateThread(
			&Producer1,
			1,
			ProducerStack1,
			STACK_SIZE,
			ProducerMain,
			(void*) &Socket1,
			0,
			TRUE );
	/*
	SchedulerCreateThread(
			&Producer2,
			1,
			ProducerStack2,
			STACK_SIZE,
			ProducerMain,
			(void*) &Socket2,
			0,
			TRUE );
	SchedulerCreateThread(
			&Producer3,
			1,
			ProducerStack3,
			STACK_SIZE,
			ProducerMain,
			(void*) &Socket3,
			0,
			TRUE );
*/

	SchedulerCreateThread(
			&Consumer1,
			1,
			ConsumerStack1,
			STACK_SIZE,
			ConsumerMain,
			&Socket1,
			0,
			TRUE );
	/*
	SchedulerCreateThread(
			&Consumer2,
			1,
			ConsumerStack2,
			STACK_SIZE,
			ConsumerMain,
			&Socket2,
			0,
			TRUE );	
	SchedulerCreateThread(
			&Consumer3,
			1,
			ConsumerStack3,
			STACK_SIZE,
			ConsumerMain,
			&Socket3,
			0,
			TRUE );
*/

	KernelStart();
	return 0;
}
