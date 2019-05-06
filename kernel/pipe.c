#include"pipe.h"

/*
 * Unit Description:
 * Provides the concept of a Pipe.
 * Pipes are a single direction byte based communication mechanism.
 * A thread can call PipeWriteBuff to put information into the pipe
 * which can be consumed by another thread through PipeReadBuff.
 *
 * Calls to pipe functions can cause threads to block.
 */

/*
 * Initializes a pipe
 */
void PipeInit( char * buff, COUNT size, struct PIPE * pipe, PIPE_READ * pr, PIPE_WRITE * pw )
{
  SemaphoreInit( & pipe->Mutex, 1 );
  SemaphoreInit( & pipe->EmptyLock, 0 ); //Buffer starts empty, block reads.
  SemaphoreInit( & pipe->FullLock, 1 );//Buffer starts empty, allow writes.
  SemaphoreInit( & pipe->ReadLock, 1 );
  SemaphoreInit( & pipe->WriteLock, 1 );
  RingBufferInit( buff, size, & pipe->Ring );
  * pr = pipe;
  * pw = pipe;
}

void PipeReadInner(SPACE * space, PIPE_READ pipe) {
  BOOL wasFull;
  BOOL dataLeft;

  // No readers can progress until there is data.
  SemaphoreDown( & pipe->EmptyLock, NULL );

  //Acqure mutex lock - No one can do any io until
  //we leave the buffer.
  SemaphoreDown( & pipe->Mutex, NULL );

  //Check and see if the buffer is full.
  //If it is, then the FullLock should
  //have been leaked, and writers should be blocking.
  wasFull = RingBufferIsFull( & pipe->Ring );
  ASSERT( wasFull ? (pipe->FullLock.Count == 0) : TRUE );

  //Perform the read.
  RingBufferReadBuffer(space, &pipe->Ring);
  // Ring is protected by a read lock which should prevent zero length reads.

  //See if the ring buffer is empty.
  //If it is then we need to leak the reader lock.
  dataLeft = !RingBufferIsEmpty( & pipe->Ring );

  //Release mutex lock - We are out of the ring, so
  //let other IO go if its already passed.
  SemaphoreUp( & pipe->Mutex );

  //If the ring was full while we have exclusive access,
  //and we  freed up some space then we should release
  //the writer lock so writers can go.
  if (wasFull > 0) {
    SemaphoreUp( & pipe->FullLock );
  }

  //If there is data left in the buffer, release the
  //empty lock so that other readers can go.
  //If there is no data in the buffer, we cant let
  //readers progress, so we leak the lock.
  if (dataLeft) {
    SemaphoreUp( & pipe->EmptyLock );
  }
}

void PipeReadBuff( SPACE * space, PIPE_READ pipe)
{
  SemaphoreDown( & pipe->ReadLock, NULL );
  PipeReadInner(space, pipe);
  SemaphoreUp( & pipe->ReadLock);
}

void PipeReadStructBuff( SPACE * space, PIPE_READ pipe)
{
  SemaphoreDown( & pipe->ReadLock, NULL );
  while (! BufferEmpty(space)) {
    PipeReadInner(space, pipe);
  }
  SemaphoreUp( & pipe->ReadLock);
  ASSERT (BufferEmpty(space));
}

void PipeWriteInner( DATA * data, PIPE_WRITE pipe ) {
  //No writers can progress until we are done.
  SemaphoreDown( & pipe->FullLock, NULL );

  //No one can do any io until we leave the buffer.
  SemaphoreDown( & pipe->Mutex, NULL );

  //Check and see if the buffer is empty.
  //If it is, then the EmptyLock should
  //have been leaked, and readers should be blocking.
  BOOL wasEmpty = RingBufferIsEmpty( & pipe->Ring );
  ASSERT( wasEmpty ? (pipe->EmptyLock.Count == 0) : TRUE );

  //Perform the write.
  RingBufferWriteBuffer(data, &pipe->Ring );
  // Ring is protected by a write lock which should prevent zero length writes.

  //See if the ring buffer is empty.
  //If it is then we need to leak the reader lock.
  BOOL spaceLeft = !RingBufferIsFull( & pipe->Ring );

  //We are out of the ring, so
  //let other IO go if its already passed.
  SemaphoreUp( & pipe->Mutex );

  //If the ring was empty while we have exclusive access,
  //and we wrote some data, then we should release
  //the EmptyLock so readers can go.
  if (wasEmpty) {
    SemaphoreUp( & pipe->EmptyLock );
  }

  //If there is space left in the buffer, release the
  //FullLock so that other writers can go.
  //If there is no space in the buffer, we cant let
  //writers progress, so we leak the lock.
  if( spaceLeft ) {
    ASSERT (BufferEmpty(data));
    SemaphoreUp( & pipe->FullLock );
  }
}

void PipeWriteBuff(DATA * buff, PIPE_WRITE pipe)
{
  SemaphoreDown( & pipe->WriteLock, NULL );
  PipeWriteInner(buff, pipe);
  SemaphoreUp( & pipe->WriteLock);
}

void PipeWriteStructBuff(DATA * data, PIPE_WRITE pipe)
{
  SemaphoreDown( & pipe->WriteLock, NULL );
  while (! BufferEmpty(data)) {
    PipeWriteInner(data, pipe);
  }
  SemaphoreUp( & pipe->WriteLock);
  ASSERT (BufferEmpty(data));
}
