#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include"utils.h"
#include"buffer.h"

struct RING_BUFFER {
  char * Buffer;
  COUNT Size;
  INDEX ReadIndex;
  INDEX WriteIndex;
  BOOL Empty;
};

COUNT RingBufferRead( char * buff, COUNT size, struct RING_BUFFER * ring ); //TODO RETIRE.
void RingBufferReadBuffer(SPACE * buff, struct RING_BUFFER * ring);
COUNT RingBufferWrite( char * buff, COUNT size, struct RING_BUFFER * ring ); //TODO RETIRE
void RingBufferWriteBuffer(DATA * buff, struct RING_BUFFER * ring);
BOOL RingBufferIsEmpty( struct RING_BUFFER * ring );
BOOL RingBufferIsFull( struct RING_BUFFER * ring );
void RingBufferInit( char * buff, COUNT size, struct RING_BUFFER * ring );

#endif
