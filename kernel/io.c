#include"io.h"
#include"socket.h"
#include"scheduler.h"
#include"hal.h"
#include"isr.h"
#include"panic.h"

/*
 * IO UNIT DESCRIPTION 
 *
 * The IO unit establishes a socket for communication over serial.
 */

//
//Interrupt variables (cross platform)
//
#define SCRATCH_SIZE 16

//Input ( from the serial port )
char InputScratch1[SCRATCH_SIZE];
char InputScratch2[SCRATCH_SIZE];
INDEX AsrInputCount;
INDEX AsrOutputCount;
INDEX ThreadInputCount;
char *AsrInputScratch;
char *ThreadInputScratch;

//Output ( to the serial port )
char OutputScratch1[SCRATCH_SIZE];
char OutputScratch2[SCRATCH_SIZE];
INDEX AsrOutputCount;//How many chars to print
INDEX AsrOutputIndex;//Which one your on.
INDEX ThreadOutputCount;
char *AsrOutputScratch;
char *ThreadOutputScratch;

//
//AVR Interrupts for print
//

#ifdef AVR_BUILD
//READ INTERRUPT
//This needs to succeed or we miss the character.
ISR(USART0_RX_vect)
{
#if 0
	char temp;
	temp = UDR; 

	IsrIncrement(IRQ_LEVEL_IO);
	if( AsrInputCount < SCRATCH_SIZE )
	{
		AsrInputScratch[AsrInputCount++] = temp;
	}
	else
	{
		KernelPanic( );
	}
	IsrDecrement(IRQ_LEVEL_IO);
#endif
}

//WRITE INTERRUPT
ISR(USART0_TX_vect)
{
#if 0 
	IsrIncrement(IRQ_LEVEL_IO);
	if( AsrOutputIndex < AsrOutputCount )
	{
		UDR = AsrOutputScratch[AsrOutputIndex++];
	}
	IsrDecrement(IRQ_LEVEL_IO);
#endif
}
#endif

//
//Read and Write Threads
//
#define IO_BUFF_SIZE 64
struct SOCKET IoSocket;
struct PIPE InputPipe;
struct PIPE OutputPipe;
char InputBuffer[IO_BUFF_SIZE];
char OutputBuffer[IO_BUFF_SIZE];

void InputMain()
{
	INDEX write = 0;
	char * temp;
	while(1)
	{
		if( write == ThreadInputCount )
		{
			//We write everything into the socket.
			//Swap the interrupt buffers.
			IsrDisable(IRQ_LEVEL_IO);

			ThreadInputCount = AsrInputCount;
			AsrInputCount = 0;
			write = 0;

			temp = ThreadInputScratch;
			ThreadInputScratch = AsrInputScratch;
			AsrInputScratch = temp;

			IsrEnable(IRQ_LEVEL_IO);
		}

		if( ThreadInputCount > 0 )
		{
			//Write data to socket.
			write = PipeWrite( ThreadInputScratch, 
					ThreadInputCount,
					&InputPipe);
		}
		else
		{
			//Nothing to write, force switch
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
	}
}

void WriteMain()
{
	char * temp;
	BOOL swap;
	while(1)
	{
		//Get some data we want to send out over serial.
		ThreadOutputCount = PipeRead( 
				ThreadOutputScratch, 
				SCRATCH_SIZE, 
				&OutputPipe );
		//See if asr is finished with last batch.
		//so we can send the new batch
		SchedulerStartCritical();
		swap = FALSE;
		while( ! swap )
		{
			IsrDisable(IRQ_LEVEL_IO);
			swap = AsrOutputIndex >= AsrOutputCount;
			IsrEnable(IRQ_LEVEL_IO);

			if( swap )
			{//Isr is ready for swap
				IsrDisable(IRQ_LEVEL_IO);

				temp = AsrOutputScratch;
				AsrOutputScratch = ThreadOutputScratch;
				ThreadOutputScratch = temp;

				AsrOutputCount = ThreadOutputCount;
				ThreadOutputCount = 0;
				AsrOutputIndex = 0;

				IsrEnable(IRQ_LEVEL_IO);
				SchedulerEndCritical();
			}
			else
			{
				SchedulerForceSwitch();
			}
		}
	}
}

void IoInit()
{
	HalSerialStartup();

	//Set up Input variables
	AsrInputCount = 0;
	ThreadInputCount = 0;
	AsrInputScratch = InputScratch1;
	ThreadInputScratch = InputScratch2;

	//Set up Output Variables
	AsrOutputCount = 0;
	ThreadOutputCount = 0;
	AsrOutputScratch = OutputScratch1;
	ThreadOutputScratch = OutputScratch2;

	//Set up Socket
	PipeInit( InputBuffer, IO_BUFF_SIZE, &InputPipe );
	PipeInit( OutputBuffer, IO_BUFF_SIZE, &OutputPipe );
	SocketInit( &InputPipe, &OutputPipe, &IoSocket );
}

//
//Public Routines for threads
//

void IoWrite( char * buff, COUNT size )
{
	SocketWriteStruct( buff, size, &IoSocket );
}

COUNT IoRead( char * buff, COUNT size )
{
	return SocketReadChars( buff, size, &IoSocket );
}

