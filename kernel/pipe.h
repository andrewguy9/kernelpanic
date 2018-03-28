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
    char * buff,
    COUNT size,
    struct PIPE * pipe,
    PIPE_READ * pr,
    PIPE_WRITE * pw );

COUNT PipeRead( char * buff, COUNT size, PIPE_READ pipe ); //TODO RETIRE.
void PipeReadBuff( SPACE * space, PIPE_READ pipe);
void PipeReadStruct( char * buff, COUNT size, PIPE_READ pipe ); //TODO RETIRE.
void PipeReadStructBuff( SPACE * space, PIPE_READ pipe);


COUNT PipeWrite( char * buff, COUNT size, PIPE_WRITE pipe );//TODO RETIRE.
void PipeWriteBuff(DATA * buff, PIPE_WRITE pipe);
void PipeWriteStruct( char * buff, COUNT size, PIPE_WRITE pipe ); //TODO RETIRE.
void PipeWriteStructBuff(DATA * buff, PIPE_WRITE pipe);

#endif
