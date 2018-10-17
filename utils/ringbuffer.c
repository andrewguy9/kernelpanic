#include"ringbuffer.h"

/*
 * Implements a ring buffer.
 */

//
//Private Routines for write
//
SPACE RingBufferSpace(struct RING_BUFFER * ring) {
  INDEX start = ring->WriteIndex;
  INDEX end;
  //find where we stop
  if (ring->WriteIndex < ring->ReadIndex) {
    //We can write from WriteIndex to ReadIndex.
    //[     w-----------r      ]
    end = ring->ReadIndex;
  } else {
    //We can write to end of buffer
    //[     r     w-----------]
    end = ring->Size;
  }
  COUNT len = end-start;
  SPACE space = BufferSpace(ring->Buffer+start, len);
  return space;
}

void RingBufferUpdateWriteIndex(SPACE * space, struct RING_BUFFER * ring) {
  //Update the WriteIndex to its future position.
  //[     c++++++ew----r     ]
  if (space->Buff == ring->Buffer + ring->Size) {
    ring->WriteIndex = 0;
  } else {
    ring->WriteIndex = (char*) space->Buff - (char*) ring->Buffer;
  }
  ring->Empty = FALSE; //TODO IS THIS ALWAYS TRUE?
}

void RingBufferWriteSmall(
    DATA * data,
    struct RING_BUFFER * ring )
{
  SPACE space = RingBufferSpace(ring);
  BufferCopy(data, &space);
  RingBufferUpdateWriteIndex(&space, ring);
}

//
//Private Routines for read
//
DATA RingBufferData(struct RING_BUFFER * ring) {
  INDEX start = ring->ReadIndex;
  INDEX end;
  //find where we stop
  if (ring->ReadIndex < ring->WriteIndex)
  {
    //We can read from ReadIndex to WriteIndex.
    //[     r-----------w      ]
    end = ring->WriteIndex;
  } else {
    //We can read to end of buffer
    //[     w     r-----------]
    end = ring->Size;
  }
  COUNT len = end-start;
  DATA data = BufferSpace(ring->Buffer+start, len);
  return data;
}

void RingBufferUpdateReadIndex(DATA * data, struct RING_BUFFER * ring) {
  //Update the ReadIndex to its future position.
  //[     c++++++er----w     ]
  if (data->Buff == ring->Buffer + ring->Size) {
    ring->ReadIndex = 0;
  } else {
    ring->ReadIndex = (char*) data->Buff - (char*) ring->Buffer;
  }
  if (ring->ReadIndex == ring->WriteIndex) {
    ring->Empty = TRUE;
  }
}

void RingBufferReadSmall(
    SPACE * space,
    struct RING_BUFFER * ring)
{
  DATA data = RingBufferData(ring);
  BufferCopy(&data, space);
  RingBufferUpdateReadIndex(&data, ring);
}

//
//Public routines
//
void RingBufferReadBuffer(SPACE * space, struct RING_BUFFER * ring)
{
  while (! BufferFull(space) && ! RingBufferIsEmpty(ring)) {
    RingBufferReadSmall(space, ring);
  }
}

void RingBufferWriteBuffer(DATA * data, struct RING_BUFFER * ring)
{
  while (! BufferEmpty(data) && ! RingBufferIsFull(ring)) {
    RingBufferWriteSmall(data, ring);
  }
}

BOOL RingBufferIsEmpty( struct RING_BUFFER * ring )
{
  return ring->Empty;
}

BOOL RingBufferIsFull( struct RING_BUFFER * ring )
{
  return ! ring->Empty && ring->WriteIndex == ring->ReadIndex;
}

void RingBufferInit( SPACE * space, struct RING_BUFFER * ring ) {
  DATA data = *space;
  BufferAdvance(&data, space);
  //TODO REACHING INTO BUFFER
  ring->Buffer = data.Buff;
  ring->Size = data.Length;
  ring->ReadIndex=0;
  ring->WriteIndex=0;
  ring->Empty = TRUE;
}
