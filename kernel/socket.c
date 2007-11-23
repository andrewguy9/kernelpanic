#include"socket.h"

void PipeInit( char * buff, COUNT size, struct PIPE * pipe )
{
	SemaphoreInit( pipe->Mutex, 1 );
	SemaphoreInit( pipe->EmptyLock, 0 );
	SemaphoreInit( pipe->FullLock, 1 );
	RingBufferInit( buff, size, pipe->Ring );
}

COUNT PipeRead( char * buff, COUNT size, struct PIPE * pipe )
{
	BOOL wasFull;
	COUNT read;

	SemaphoreDown( lock->EmptyLock );
	SemaphoreDown( lock->Mutex);
	wasFull = RingBufferIsFull( pipe->Ring ); 
	read = RingBufferRead( buff, size, pipe->Ring );
	SemaphoreUp( lock->Mutex );
	if( wasFull )
		SemaphoreUp( pipe->FullLock );
	return read;
}

COUNT PipeWrite( char * buff, COUNT size, struct PIPE * pipe )
{
	BOOL wasEmtpy;
	COUNT write;
	SemaphoreDown( pipe->FullLock );
	SemaphoreDown( pipe->Mutex );
	wasEmpty = RingBufferIsEmpty( pipe->Ring );
	write = RingBufferWrite( buff, size, pipe->Ring );
	SemaphoreUp( pipe->Mutex );
	if( wasEmpty )
		SemaphoreUp( pipe->EmptyLock );
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
