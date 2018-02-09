#include"socket.h"

/*
 * Unit Description:
 * Defines a Socket structure for threads.
 * Sockets are like pipes except that they are bi-directional.
 */

void SocketInit( struct PIPE * readPipe, struct PIPE * writePipe, struct SOCKET * socket )
{
        socket->ReadPipe = readPipe;
        socket->WritePipe = writePipe;
}

COUNT SocketReadChars( char * buff, COUNT size, struct SOCKET * socket )
{
        return PipeRead( buff, size, socket->ReadPipe );
}

void SocketReadStruct( char * buff, COUNT size, struct SOCKET * socket )
{
        PipeReadStruct( buff, size, socket->ReadPipe );
}

COUNT SocketWriteChars( char * buff, COUNT size, struct SOCKET * socket )
{
        return PipeWrite( buff, size, socket->WritePipe );
}

void SocketWriteStruct( char * buff, COUNT size, struct SOCKET * socket )
{
        PipeWriteStruct( buff, size, socket->WritePipe );
}
