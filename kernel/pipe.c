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
	SemaphoreInit( & pipe->ReaderLock, 0 ); //Buffer starts empty, to it is not readable. Semaphore starts locked
	SemaphoreInit( & pipe->WriterLock, 1 );//Buffer starts empty, so it is writable. Semaphore started unlocked.
	RingBufferInit( buff, size, & pipe->Ring );
}

/*
 * Reads data from a pipe.
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

	//Acquire ReaderLock - No readers can progress until we are done.
	SemaphoreDown( & pipe->ReaderLock, NULL );

	//Acqure mutex lock - No one can do any io until 
	//we leave the buffer.
	SemaphoreDown( & pipe->Mutex, NULL );

	//Check and see if the buffer if full.
	//If it is, then the writer lock should
	//have been leaked, and writers should be blocking.
	wasFull = RingBufferIsFull( & pipe->Ring ); 
	ASSERT( wasFull ? (pipe->WriterLock.Count == 0) : TRUE );

	//Perform the read.
	read = RingBufferRead( buff, size, & pipe->Ring );

	//See if the ring buffer is empty.
	//If it is then we need to leak the reader lock.
	dataLeft = !RingBufferIsEmpty( & pipe->Ring );

	//Release mutex lock - We are out of the ring, so
	//let other IO go if its already passed.
	SemaphoreUp( & pipe->Mutex );

	//If the ring was full while we have exclusive access,
	//and we  freed up some space then we should release 
	//the writer lock so writers can go.
	if( wasFull && read > 0 )
		SemaphoreUp( & pipe->WriterLock );

	//If there is data left in the buffer, release the 
	//empty lock so that other readers can go.
	//If there is no data in the buffer, we cant let
	//readers progress, so we leak the lock. 
	if( dataLeft )
		SemaphoreUp( & pipe->ReaderLock );

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
	COUNT write;

	//Acquire WriterLock - No readers can progress until we are done.
	SemaphoreDown( & pipe->WriterLock, NULL );

	//Acqure mutex lock - No one can do any io until 
	//we leave the buffer.
	SemaphoreDown( & pipe->Mutex, NULL );

	//Check and see if the buffer if full.
	//If it is, then the writer lock should
	//have been leaked, and writers should be blocking.
	wasEmpty = RingBufferIsEmpty( & pipe->Ring ); 
	ASSERT( wasEmpty ? (pipe->ReaderLock.Count == 0) : TRUE );

	//Perform the write.
	write = RingBufferWrite( buff, size, & pipe->Ring );

	//See if the ring buffer is empty.
	//If it is then we need to leak the reader lock.
	spaceLeft = !RingBufferIsFull( & pipe->Ring );

	//Release mutex lock - We are out of the ring, so
	//let other IO go if its already passed.
	SemaphoreUp( & pipe->Mutex );

	//If the ring was empty while we have exclusive access,
	//and we wrote some data, then we should release 
	//the reader lock so readers can go.
	if( wasEmpty && write > 0 )
		SemaphoreUp( & pipe->ReaderLock );

	//If there is space left in the buffer, release the 
	//writer lock so that other writers can go.
	//If there is no space in the buffer, we cant let
	//writers progress, so we leak the lock. 
	if( spaceLeft )
		SemaphoreUp( & pipe->WriterLock );

	return write;
}

