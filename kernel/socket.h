#ifndef SOCKET_H
#define SOCKET_H

#include"semaphore.h"
#include"pipe.h"
#include"utils/utils.h"

struct SOCKET
{
  struct PIPE Pipe1;
  struct PIPE Pipe2;
};

struct SOCKET_HANDLE {
  PIPE_READ Read;
  PIPE_WRITE Write;
};

void SocketInit(
    char * buff1,
    COUNT buff1_size,
    char * buff2,
    COUNT buff2_size,
    struct SOCKET * socket,
    struct SOCKET_HANDLE * h1,
    struct SOCKET_HANDLE * h2
    );

COUNT SocketReadChars( char * buff, COUNT size, struct SOCKET_HANDLE * socket ); //TODO RETIRE
void SocketReadCharsBuffer( SPACE * space, struct SOCKET_HANDLE * socket );
void SocketReadStruct( char * buff, COUNT size, struct SOCKET_HANDLE * socket ); //TODO RETIRE
void SocketReadStructBuffer( SPACE * space, struct SOCKET_HANDLE * socket );
COUNT SocketWriteChars( char * buff, COUNT size, struct SOCKET_HANDLE * socket ); //TODO RETIRE
void SocketWriteCharsBuffer( DATA * data, struct SOCKET_HANDLE * socket );
void SocketWriteStruct( char * buff, COUNT size, struct SOCKET_HANDLE * socket ); //TODO RETIRE
void SocketWriteStructBuffer( DATA * data, struct SOCKET_HANDLE * socket );

#endif
