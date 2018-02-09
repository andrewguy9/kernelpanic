#ifndef SOCKET_H
#define SOCKET_H

#include"semaphore.h"
#include"pipe.h"
#include"utils/utils.h"

struct SOCKET
{
        PIPE_READ ReadPipe;
        PIPE_WRITE WritePipe;
};

void SocketInit( struct PIPE * readPipe, struct PIPE * writePipe, struct SOCKET * socket );
COUNT SocketReadChars( char * buff, COUNT size, struct SOCKET * socket );
void SocketReadStruct( char * buff, COUNT size, struct SOCKET * socket );
COUNT SocketWriteChars( char * buff, COUNT size, struct SOCKET * socket );
void SocketWriteStruct( char * buff, COUNT size, struct SOCKET * socket );
#endif
