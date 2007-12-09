#include"socket.h"

/*
 * Unit Description:
 * Defines a Socket structure for threads.
 */

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
		read += PipeRead( buff+read, size-read, socket->ReadPipe );
	SemaphoreUp( & socket->ReadLock );
	return read;
}

COUNT SocketWrite( char * buff, COUNT size, struct SOCKET * socket )
{
	COUNT write=0;
	SemaphoreDown( & socket->WriteLock );
	while( write < size )
		write += PipeWrite( buff+write, size-write, socket->WritePipe );
	SemaphoreUp( & socket->WriteLock );
	return write;
}
