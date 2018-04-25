#include"socket.h"

/*
 * Unit Description:
 * Defines a Socket structure for threads.
 * Sockets are like pipes except that they are bi-directional.
 */

void SocketInit(
    SPACE * space1,
    SPACE * space2,
    struct SOCKET * socket,
    struct SOCKET_HANDLE * h1,
    struct SOCKET_HANDLE * h2
    ) {
  PIPE_READ p1_read;
  PIPE_WRITE p1_write;
  PIPE_READ p2_read;
  PIPE_WRITE p2_write;

  PipeInit(space1, &socket->Pipe1, &p1_read, &p1_write);
  PipeInit(space2, &socket->Pipe2, &p2_read, &p2_write);
  h1->Read = p1_read;
  h1->Write = p2_write;
  h2->Read = p2_read;
  h2->Write = p1_write;
}

void SocketReadCharsBuffer( SPACE * space, struct SOCKET_HANDLE * socket )
{
  PipeReadBuffer(space, socket->Read);
}

void SocketReadStructBuffer( SPACE * space, struct SOCKET_HANDLE * socket )
{
  PipeReadStructBuffer(space, socket->Read);
}

void SocketWriteCharsBuffer( DATA * data, struct SOCKET_HANDLE * socket )
{
  PipeWriteBuffer(data, socket->Write);
}

void SocketWriteStructBuffer( DATA * data, struct SOCKET_HANDLE * socket )
{
  PipeWriteStructBuffer(data, socket->Write);
}
