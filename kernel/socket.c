#include"socket.h"

/*
 * Unit Description:
 * Defines a Socket structure for threads.
 */

void PipeInit( char * buff, COUNT size, struct PIPE * pipe )
{
	SemaphoreInit( pipe->Mutex, 1 );
	SemaphoreInit( pipe->EmptyLock, 0 ); //>0 when readable, <=0 when empty
	SemaphoreInit( pipe->FullLock, 1 );//>0 when writable, <= when full
	RingBufferInit( buff, size, pipe->Ring );
}

COUNT PipeRead( char * buff, COUNT size, struct PIPE * pipe )
{
	BOOL wasFull;
	BOOL dataLeft;
	COUNT read;

	//Acquire locks
	SemaphoreDown( lock->EmptyLock );
	SemaphoreDown( lock->Mutex);
	//Perform read
	wasFull = RingBufferIsFull( pipe->Ring ); 
	read = RingBufferRead( buff, size, pipe->Ring );
	dataLeft = ! RingBufferIsEmpty( pipe->Ring );
	//Release locks
	SemaphoreUp( lock->Mutex );
	if( dataLeft )
		SemaphoreUp( pipe->EmptyLock );
	if( wasFull && read > 0)
		SemaphoreUp( pipe->FullLock );
	//Return result
	return read;
}

COUNT PipeWrite( char * buff, COUNT size, struct PIPE * pipe )
{
	BOOL wasEmtpy;
	BOOL spaceLeft;
	COUNT write;

	//Acquire locks
	SemaphoreDown( pipe->FullLock );
	SemaphoreDown( pipe->Mutex );
	//Perform write
	wasEmpty = RingBufferIsEmpty( pipe->Ring );
	write = RingBufferWrite( buff, size, pipe->Ring );
	spaceLeft = RingBufferIsFull( pipe->Ring );
	//Release locks
	SemaphoreUp( pipe->Mutex );
	if( spaceLeft )
		SemaphoreUp( pipe->FullLock );
	if( wasEmpty && write > 0 )
		SemaphoreUp( pipe->EmptyLock );
	//Return result
	return write;
}

void SocketInit( struct PIPE * readPipe, struct PIPE * writePipe, struct SOCKET * socket )
{
	socket->ReadPipe = readPipe;
	socket->WritePipe = writePipe;
}

COUNT SocketRead( char * buff, COUNT size, struct SOCKET * socket )
{
	return PipeRead( buff, size, socket->ReadPipe );
}

COUNT SocketWrite( char * buff, COUNT size, struct SOCKET * socket )
{
	return PipeWrite( buff, size, socket->WritePipe );
}
