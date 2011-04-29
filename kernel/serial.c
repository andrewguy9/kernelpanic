#include"serial.h"
#include"hal.h"
#include"critinterrupt.h"
#include"../utils/ringbuffer.h"
#include"signal.h"

#include<stdio.h>

#define BUFFER_SIZE 32

char SerialInputBuffer[BUFFER_SIZE];
char SerialOutputBuffer[BUFFER_SIZE];

struct RING_BUFFER SerialInputRing;
struct RING_BUFFER SerialOutputRing;

struct HANDLER_OBJECT GetBytesCritObject;

struct SIGNAL GetBytesSignal;

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

BOOL GetBytesCritHandler(struct HANDLER_OBJECT * handler)
{
	IsrDisable(IRQ_LEVEL_SERIAL_WRITE);

	while(!RingBufferIsFull(&SerialInputRing)) {
		char data;
		if(HalSerialGetChar(&data)) {
			ASSUME(RingBufferWrite(&data, sizeof(data), &SerialInputRing), 1);
		} else {
			break;
		}
	}

	IsrEnable(IRQ_LEVEL_SERIAL_WRITE);
	
	// Here there is a race between us adding the new bytes and someone 
	// taking them out before we signal. This can lead to spurious signaling.
	// On this side we don't care if we signal too often because at worst we 
	// trick someone into reading a zero byte read.
	SignalSet( &GetBytesSignal );
	return TRUE;
}

void GetBytesInterrupt(void)
{
	IsrIncrement(IRQ_LEVEL_SERIAL_READ);

	if( HandlerIsFinished(&GetBytesCritObject) )
		CritInterruptRegisterHandler( &GetBytesCritObject, GetBytesCritHandler, NULL );

	IsrDecrement(IRQ_LEVEL_SERIAL_READ);
}

void SerialStartup()
{
	RingBufferInit( SerialInputBuffer, BUFFER_SIZE, &SerialInputRing );
	RingBufferInit( SerialOutputBuffer, BUFFER_SIZE, &SerialOutputRing );

	HandlerInit( &GetBytesCritObject );
	
	SignalInit( &GetBytesSignal, FALSE );

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
	COUNT read = 0;
	BOOL wasFull = FALSE;

	do {
		BOOL empty = FALSE;

		IsrDisable(IRQ_LEVEL_SERIAL_READ);

		if( RingBufferIsFull( &SerialInputRing ) ) {
			wasFull = TRUE;
		}

		read = RingBufferRead(buf, len, &SerialInputRing);
		if( RingBufferIsEmpty( &SerialInputRing ) ) {
			empty = TRUE;
		}

		IsrEnable(IRQ_LEVEL_SERIAL_READ);

		// Here there is a race between us adding the new bytes and someone 
		// taking them out before we signal. This can lead to spurious signaling.
		// We care about spurious signaling on this side because we can accidentally cause
		// all readers to block when there is data available. This is why we always attempt
		// a read before blocking on zero data. THIS DOES NOT FIX THE RACE, it just makes
		// it less of an issue when it happens.
		if( empty ) {
			SignalUnset( &GetBytesSignal );
		}

		if(read == 0) {
			SignalWaitForSignal( &GetBytesSignal, NULL );
		}
	} while(read == 0);

	// We were full, so lets make sure there wasn't any data buffered
	// in the hal.
	if( wasFull ) {
		HalRaiseInterrupt( IRQ_LEVEL_SERIAL_READ );
	}
	return read;
}

