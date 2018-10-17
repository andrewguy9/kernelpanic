#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include"utils.h"
#include"buffer.h"

struct RING_BUFFER {
  void * Buffer;
  COUNT Size;
  INDEX ReadIndex;
  INDEX WriteIndex;
  BOOL Empty;
};

void RingBufferReadBuffer(SPACE * space, struct RING_BUFFER * ring);
void RingBufferWriteBuffer(DATA * data, struct RING_BUFFER * ring);
BOOL RingBufferIsEmpty( struct RING_BUFFER * ring );
BOOL RingBufferIsFull( struct RING_BUFFER * ring );
void RingBufferInit( SPACE * space, struct RING_BUFFER * ring );

#endif
