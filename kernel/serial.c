#include"serial.h"
#include"hal.h"
#include"critinterrupt.h"
#include"../utils/ringbuffer.h"

#include<stdio.h>

#define BUFFER_SIZE 32

char SerialInputBuffer[BUFFER_SIZE];
char SerialOutputBuffer[BUFFER_SIZE];

struct RING_BUFFER SerialInputRing;
struct RING_BUFFER SerialOutputRing;

void SendBytesInterrupt(void)
{
	IsrIncrement(IRQ_LEVEL_SERIAL_WRITE);

	ASSERT(IsrIsAtomic(IRQ_LEVEL_SERIAL_WRITE));
	while(!RingBufferIsEmpty(&SerialOutputRing)) {
		char data;
		ASSUME(RingBufferRead(&data, sizeof(data), &SerialOutputRing), 1);
		HalSerialWriteChar(data);
	}

	IsrDecrement(IRQ_LEVEL_SERIAL_WRITE);
}

void GetBytesInterrupt(void)
{
	IsrIncrement(IRQ_LEVEL_SERIAL_READ);

	ASSERT(IsrIsAtomic(IRQ_LEVEL_SERIAL_READ));
	while(!RingBufferIsFull(&SerialInputRing)) {

		char data;
		if(HalSerialGetChar(&data)) {
			ASSUME(RingBufferWrite(&data, sizeof(data), &SerialInputRing), 1);
		} else {
			break;
		}
	}

	IsrDecrement(IRQ_LEVEL_SERIAL_READ);
}

void SerialStartup()
{
	RingBufferInit( SerialInputBuffer, BUFFER_SIZE, &SerialInputRing );
	RingBufferInit( SerialOutputBuffer, BUFFER_SIZE, &SerialOutputRing );
	
	HalRegisterIsrHandler( SendBytesInterrupt, (void *) HAL_ISR_SERIAL_WRITE, IRQ_LEVEL_SERIAL_WRITE);
	HalRegisterIsrHandler( GetBytesInterrupt, (void *) HAL_ISR_SERIAL_READ, IRQ_LEVEL_SERIAL_READ);

	HalStartSerial();
}

COUNT SerialWrite(char * buf, COUNT len)
{
	COUNT write;

	IsrDisable(IRQ_LEVEL_SERIAL_WRITE);
	write = RingBufferWrite(buf, len, &SerialOutputRing);
	IsrEnable(IRQ_LEVEL_SERIAL_WRITE);

	HalRaiseInterrupt(IRQ_LEVEL_SERIAL_WRITE);
	
	return write;
}

COUNT SerialRead(char * buf, COUNT len)
{
	COUNT read;

	IsrDisable(IRQ_LEVEL_SERIAL_READ);
	read = RingBufferRead(buf, len, &SerialInputRing);
	IsrEnable(IRQ_LEVEL_SERIAL_READ);

	return read;
}

