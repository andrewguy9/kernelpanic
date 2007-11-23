#ifndef SOCKET_H
#define SOCKET_H

#include"../utils/ringbuffer.h"
#include"semaphore.h"
#include"../utils/utils.h"

struct PIPE{
	struct RING_BUFFER Ring;
	struct SEMAPHORE Mutex;
	struct SEMAPHORE EmptyLock;
	struct SEMAPHORE FullLock;
};

void PipeInit( char * buff, COUNT size, struct PIPE * pipe );
COUNT PipeRead( char * buff, COUNT size, struct PIPE * pipe );
COUNT PipeWrite( char * buff, COUNT size, struct PIPE * pipe );

struct SOCKET {
	struct PIPE * ReadPipe;
	struct PIPE * WritePipe;
};

void SocketInit( struct PIPE * readPipe, struct PIPE * writePipe, struct SOCKET * socket );
COUNT SocketRead( char * buff, COUNT size, struct SOCKET * socket );
COUNT SocketWrite( char * buff, COUNT size, struct SOCKET * socket );

#endif
