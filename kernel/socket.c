#include"socket.h"


void PipeInit( char * buff, COUNT size, struct PIPE * pipe )
{
	SemaphoreInit( pipe->Lock, 1 );
	RingBufferInit( buff, size, pipe->Ring );
}

COUNT PipeRead( char * buff, COUNT size, struct PIPE * pipe )
{
	COUNT read;
	SemaphoreLock( pipe->Lock, 1 );
	read = RingBufferRead( buff, size, pipe->Ring );
	SemaphoreUnlock( pipe->Lock, 1);
	return read;
}

COUNT PipeWrite( char * buff, COUNT size, struct PIPE * pipe )
{
	COUNT write;
	SemaphoreLock(pipe->Lock, 1 );
	write = RingBufferWrite( buff, size, pipe );
	SemaphoreUnlock( pipe->Lock, 1 );
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
