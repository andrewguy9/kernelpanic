#include"pipe.h"

/*
 * Unit Description:
 * Provides the concept of a Pipe.
 * Pipes are a single direction byte based communication mechanism. 
 * A thread can call PipeWrite to put information into the pipe
 * which can be consumed by another thread through PipeRead.
 *
 * Calls to pipe functions can cause threads to block.
 */

/*
 * Initializes a pipe
 */
void PipeInit( char * buff, COUNT size, struct PIPE * pipe )
{
	SemaphoreInit( & pipe->Mutex, 1 );
	SemaphoreInit( & pipe->EmptyLock, 0 ); //>0 when readable, <=0 when empty
	SemaphoreInit( & pipe->FullLock, 1 );//>0 when writable, <= when full
	RingBufferInit( buff, size, & pipe->Ring );
}

/*
 * Reads data froma pipe.
 *
 * Arguments:
 * buff - destination buffer
 * size - maximum length that will be read.
 * pipe - pipe we will read from
 *
 * Returns
 * The length of data we read. 
 *
 * The size of the read data will be min( data in pipe, size)
 */
COUNT PipeRead( char * buff, COUNT size, struct PIPE * pipe )
{
	BOOL wasFull;
	BOOL dataLeft;
	COUNT read;

	//Acquire locks
	SemaphoreDown( & pipe->EmptyLock, NULL );
	SemaphoreDown( & pipe->Mutex, NULL );
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

/*
 * Writes data to a pipe.
 *
 * Arguments
 * buff - buffer we will read from
 * size - the maximum distance we will read from.
 * pipe - the pipe we will copy data to.
 *
 * Returns
 * The length of data we read from buff.
 *
 * The size of the written data will be min( space left in pipe, size)
 */
COUNT PipeWrite( char * buff, COUNT size, struct PIPE * pipe )
{
	BOOL wasEmpty;
	BOOL spaceLeft;
	COUNT write=0;

	//Acquire locks
	SemaphoreDown( & pipe->FullLock, NULL ); //check not full
	SemaphoreDown( & pipe->Mutex, NULL ); //exclusive access
	//Perform write
	wasEmpty = RingBufferIsEmpty( & pipe->Ring );
	write = RingBufferWrite( buff, size, & pipe->Ring );
	spaceLeft = ! RingBufferIsFull( & pipe->Ring );
	//Release locks
	SemaphoreUp( & pipe->Mutex );//end exclusive hold
	if( spaceLeft )
		SemaphoreUp( & pipe->FullLock );//pipe not full
	if( wasEmpty && write > 0 )
		SemaphoreUp( & pipe->EmptyLock );//pipe has data(wake reader)
	//Return result
	return write;
}
