#ifndef SOCKET_H
#define SOCKET_H

#include"semaphore.h"
#include"pipe.h"
#include"../utils/utils.h"

struct SOCKET
{
	struct PIPE * ReadPipe;
	struct PIPE * WritePipe;
	struct SEMAPHORE ReadLock;
	struct SEMAPHORE WriteLock;
};

void SocketInit( struct PIPE * readPipe, struct PIPE * writePipe, struct SOCKET * socket );
COUNT SocketReadChars( char * buff, COUNT size, struct SOCKET * socket );
COUNT SocketReadStruct( char * buff, COUNT size, struct SOCKET * socket );
COUNT SocketWriteChars( char * buff, COUNT size, struct SOCKET * socket );
void SocketWriteStruct( char * buff, COUNT size, struct SOCKET * socket );
#endif
