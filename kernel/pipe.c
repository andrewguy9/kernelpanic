#include"pipe.h"

/*
 * Unit Description:
 * Defines a pipe structure for theads
 */

void PipeInit( char * buff, COUNT size, struct PIPE * pipe )
{
	SemaphoreInit( & pipe->Mutex, 1 );
	SemaphoreInit( & pipe->EmptyLock, 0 ); //>0 when readable, <=0 when empty
	SemaphoreInit( & pipe->FullLock, 1 );//>0 when writable, <= when full
	RingBufferInit( buff, size, & pipe->Ring );
}

COUNT PipeRead( char * buff, COUNT size, struct PIPE * pipe )
{
	BOOL wasFull;
	BOOL dataLeft;
	COUNT read;

	//Acquire locks
	SemaphoreDown( & pipe->EmptyLock );
	SemaphoreDown( & pipe->Mutex);
	//Perform read
	wasFull = RingBufferIsFull( & pipe->Ring ); 
	read = RingBufferRead( buff, size, & pipe->Ring );
	dataLeft = ! RingBufferIsEmpty( & pipe->Ring );
	//Release locks
	SemaphoreUp( & pipe->Mutex );
	if( dataLeft )
		SemaphoreUp( & pipe->EmptyLock );
	if( wasFull && read > 0)
		SemaphoreUp( & pipe->FullLock );
	//Return result
	return read;
}

COUNT PipeWrite( char * buff, COUNT size, struct PIPE * pipe )
{
	BOOL wasEmpty;
	BOOL spaceLeft;
	COUNT write=0;

	//Acquire locks
	SemaphoreDown( & pipe->FullLock ); //check not full
	SemaphoreDown( & pipe->Mutex ); //exclusive access
	//Perform write
	wasEmpty = RingBufferIsEmpty( & pipe->Ring );
	write = RingBufferWrite( buff, size, & pipe->Ring );
	spaceLeft = RingBufferIsFull( & pipe->Ring );
	//Release locks
	SemaphoreUp( & pipe->Mutex );//end exclusive hold
	if( spaceLeft )
		SemaphoreUp( & pipe->FullLock );//pipe not full
	if( wasEmpty && write > 0 )
		SemaphoreUp( & pipe->EmptyLock );//pipe has data(wake reader)
	//Return result
	return write;
}
