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
    SPACE * space1,
    SPACE * space2,
    struct SOCKET * socket,
    struct SOCKET_HANDLE * h1,
    struct SOCKET_HANDLE * h2
    );

void SocketReadCharsBuffer( SPACE * space, struct SOCKET_HANDLE * socket );
void SocketReadStructBuffer( SPACE * space, struct SOCKET_HANDLE * socket );
void SocketWriteCharsBuffer( DATA * data, struct SOCKET_HANDLE * socket );
void SocketWriteStructBuffer( DATA * data, struct SOCKET_HANDLE * socket );

#endif
