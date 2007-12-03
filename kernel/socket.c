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
	COUNT write=0;

	//Acquire locks
	SemaphoreDown( pipe->FullLock ); //check not full
	SemaphoreDown( pipe->Mutex ); //exclusive access
	//Perform write
	wasEmpty = RingBufferIsEmpty( pipe->Ring );
	write = RingBufferWrite( buff, size, pipe->Ring );
	spaceLeft = RingBufferIsFull( pipe->Ring );
	//Release locks
	SemaphoreUp( pipe->Mutex );//end exclusive hold
	if( spaceLeft )
		SemaphoreUp( pipe->FullLock );//pipe not full
	if( wasEmpty && write > 0 )
		SemaphoreUp( pipe->EmptyLock );//pipe has data(wake reader)
	//Return result
	return write;
}

void SocketInit( struct PIPE * readPipe, struct PIPE * writePipe, struct SOCKET * socket )
{
	socket->ReadPipe = readPipe;
	socket->WritePipe = writePipe;
	SemaphoreInit( & socket->ReadLock, 1 );
	SemaphoreInit( & socket->WriteLock, 1 );
}

COUNT SocketReadChars( char * buff, COUNT size, struct SOCKET * socket )
{
	COUNT read;
	SemaphoreDown( & socket->ReadLock );
	read = PipeRead( buff, size, socket->ReadPipe );
	SemaphoreUp( & socket->WriteLock );
	return read;
}

COUNT SocketReadStruct( char * buff, COUNT size, struct SOCKET * socket )
{
	COUNT read = 0;
	SemaphoreDown( & socket->ReadLock );
	while( read < size )
		read += PipeRead( buff, size, socket->ReadPipe );
	SemaphoreUp( & socket->ReadLock );
	return COUNT;
}

COUNT SocketWrite( char * buff, COUNT size, struct SOCKET * socket )
{
	COUNT write=0;
	SemaphoreDown( & socket->WriteLock );
	while( write < size )
		write += PipeWrite( buff, size, socket->WritePipe );
	SemaphoreUp( & socket->WriteLock );
	return write;
}
