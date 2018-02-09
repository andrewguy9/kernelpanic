#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/pipe.h"
#include"kernel/panic.h"
#include"kernel/socket.h"
#include"kernel/watchdog.h"
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

#define STACK_SIZE HAL_MIN_STACK_SIZE

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

struct THREAD_CONTEXT
{
        INDEX WatchdogId;
        struct SOCKET * Socket;
};

struct THREAD_CONTEXT ProducerContext1 = {1, &Socket1};
struct THREAD_CONTEXT ProducerContext2 = {2, &Socket2};
struct THREAD_CONTEXT ProducerContext3 = {3, &Socket3};
struct THREAD_CONTEXT ConsumerContext1 = {4, &Socket1};
struct THREAD_CONTEXT ConsumerContext2 = {5, &Socket2};
struct THREAD_CONTEXT ConsumerContext3 = {6, &Socket3};

#define QUANTUM 1
#define TIMEOUT (2*QUANTUM*6)
volatile COUNT ProducerCount;
volatile COUNT ConsumerCount;

THREAD_MAIN ProducerMain;
void ProducerMain( void * arg )
{
        struct THREAD_CONTEXT * context = (struct THREAD_CONTEXT *) arg;
	struct SOCKET * MySock = context->Socket;
	INDEX timeIndex;
	INDEX bufferIndex;
	COUNT length;
	char AssendingBuffer[RANDOM_VALUES_SIZE];
	char DecendingBuffer[RANDOM_VALUES_SIZE];
	BOOL assending;

	char * curBuffer;

        WatchdogAddFlag(context->WatchdogId);

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
                WatchdogNotify(context->WatchdogId);
	}
}

THREAD_MAIN ConsumerMain;
void ConsumerMain( void * arg )
{
        struct THREAD_CONTEXT * context = (struct THREAD_CONTEXT *) arg;
	struct SOCKET * MySock = context->Socket;
	INDEX timeIndex;
	COUNT bufferIndex;
	char myBuffer[RANDOM_VALUES_SIZE];
	BOOL assending;
	COUNT length;

	timeIndex = 0;
	assending = TRUE;

        WatchdogAddFlag(context->WatchdogId);

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
                WatchdogNotify(context->WatchdogId);
	}
}

void SetupSocket(char * buff, struct PIPE * pipe, struct SOCKET * socket) {
        PIPE_READ reader;
        PIPE_WRITE writer;

        PipeInit( buff, BUFFER_LENGTH, pipe, &reader, &writer );
        SocketInit( reader, writer, socket);
}

int main()
{
        KernelInit();

        SchedulerStartup();

        ConsumerCount = 0;
        ProducerCount = 0;

        SetupSocket(MessageBuffer1, &Pipe1, &Socket1);
        SetupSocket(MessageBuffer2, &Pipe2, &Socket2);
        SetupSocket(MessageBuffer3, &Pipe3, &Socket3);

        SchedulerCreateThread(
                        &Producer1,
                        QUANTUM,
                        ProducerStack1,
                        STACK_SIZE,
                        ProducerMain,
                        & ProducerContext1,
                        TRUE );
        SchedulerCreateThread(
                        &Producer2,
                        QUANTUM,
                        ProducerStack2,
                        STACK_SIZE,
                        ProducerMain,
                        & ProducerContext2,
                        TRUE );
        SchedulerCreateThread(
                        &Producer3,
                        QUANTUM,
                        ProducerStack3,
                        STACK_SIZE,
                        ProducerMain,
                        & ProducerContext3,
                        TRUE );
        SchedulerCreateThread(
                        &Consumer1,
                        QUANTUM,
                        ConsumerStack1,
                        STACK_SIZE,
                        ConsumerMain,
                        & ConsumerContext1,
                        TRUE );
        SchedulerCreateThread(
                        &Consumer2,
                        QUANTUM,
                        ConsumerStack2,
                        STACK_SIZE,
                        ConsumerMain,
                        & ConsumerContext2,
                        TRUE );
        SchedulerCreateThread(
                        &Consumer3,
                        QUANTUM,
                        ConsumerStack3,
                        STACK_SIZE,
                        ConsumerMain,
                        & ConsumerContext3,
                        TRUE );

        WatchdogEnable( TIMEOUT );
        KernelStart();
        return 0;
}
