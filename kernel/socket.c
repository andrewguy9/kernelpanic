#include"socket.h"

/*
 * Unit Description:
 * Defines a Socket structure for threads.
 * Sockets are like pipes except that they are bi-directional.
 */

void SocketInit(
    char * buff1,
    COUNT buff1_size,
    char * buff2,
    COUNT buff2_size,
    struct SOCKET * socket,
    struct SOCKET_HANDLE * h1,
    struct SOCKET_HANDLE * h2
    ) {
  PIPE_READ p1_read;
  PIPE_WRITE p1_write;
  PIPE_READ p2_read;
  PIPE_WRITE p2_write;

  PipeInit(buff1, buff1_size, &socket->Pipe1, &p1_read, &p1_write);
  PipeInit(buff2, buff2_size, &socket->Pipe2, &p2_read, &p2_write);
  h1->Read = p1_read;
  h1->Write = p2_write;
  h2->Read = p2_read;
  h2->Write = p1_write;
}

COUNT SocketReadChars( char * buff, COUNT size, struct SOCKET_HANDLE * socket )
{
  return PipeRead( buff, size, socket->Read);
}

void SocketReadStruct( char * buff, COUNT size, struct SOCKET_HANDLE * socket )
{
  PipeReadStruct( buff, size, socket->Read);
}

COUNT SocketWriteChars( char * buff, COUNT size, struct SOCKET_HANDLE * socket )
{
  return PipeWrite( buff, size, socket->Write);
}

void SocketWriteStruct( char * buff, COUNT size, struct SOCKET_HANDLE * socket )
{
  PipeWriteStruct( buff, size, socket->Write);
}
