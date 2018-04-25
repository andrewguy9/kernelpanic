#include"utils/ringbuffer.h"
#include"semaphore.h"
#include"utils/utils.h"

#ifndef PIPE_H
#define PIPE_H

struct PIPE
{
  struct RING_BUFFER Ring;
  struct SEMAPHORE Mutex;
  struct SEMAPHORE EmptyLock;
  struct SEMAPHORE FullLock;
  struct SEMAPHORE ReadLock;
  struct SEMAPHORE WriteLock;
};

typedef struct PIPE * PIPE_READ;
typedef struct PIPE * PIPE_WRITE;
void PipeInit(
    SPACE * space,
    struct PIPE * pipe,
    PIPE_READ * pr,
    PIPE_WRITE * pw );

void PipeReadBuffer( SPACE * space, PIPE_READ pipe);
void PipeReadStructBuffer( SPACE * space, PIPE_READ pipe);


void PipeWriteBuffer(DATA * buff, PIPE_WRITE pipe);
void PipeWriteStructBuffer(DATA * buff, PIPE_WRITE pipe);

#endif
