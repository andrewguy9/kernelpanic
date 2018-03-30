#include"ringbuffer.h"

/*
 * Implements a ring buffer.
 */

//
//Private Routines
//

void RingBufferReadSmall(
    SPACE * space,
    struct RING_BUFFER * ring)
{
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
  BufferCopy(&data, space);
  //Update the ReadIndex to its future position.
  //[     c++++++er----w     ]
  if (data.Buff == ring->Buffer + ring->Size) {
    ring->ReadIndex = 0;
  } else {
    ring->ReadIndex = data.Buff - ring->Buffer;
  }
  if (ring->ReadIndex == ring->WriteIndex) {
    ring->Empty = TRUE;
  }
}

//TODO MAKE A BUFFER IMPLEMENTATION.
COUNT RingBufferWriteSmall( char *buff, COUNT size, struct RING_BUFFER * ring )
{
  INDEX start = ring->WriteIndex;
  INDEX end = ring->WriteIndex + size;
  //find where we stop
  if (ring->WriteIndex < ring->ReadIndex) {
    //We can write to the read index only
    //[     w-----------r???e  ]
    if( end > ring->ReadIndex ) {
      //Make sure we don't go past ReadIndex
      //[     w----------er     ]
      end = ring->ReadIndex;
    }
    //Update the WriteIndex to its future position.
    //[     c++++++ew----r     ]
    ring->WriteIndex = end;
  } else {
    //We can write to end of buffer
    //[     r     w-----------]???e
    if (end >= ring->Size) {
      //Make sure we don't write past end of buffer
      //[     r     w----------e]
      end = ring->Size;
      //Wrap teh WriteIndex back to 0;
      //[w    r     c----------e]
      ring->WriteIndex = 0;
    } else {
      //Write isn't to end, so don't wrap.
      //[     r     w-------e---]
      ring->WriteIndex = end;
    }
  }
  COUNT len = end-start;
  SPACE space = BufferSpace(ring->Buffer+start, len);
  DATA data = BufferSpace(buff, len);
  BufferCopy(&data, &space);
  return end-start;
}

//
//Public routines
//
COUNT RingBufferRead( char * buff, COUNT size, struct RING_BUFFER * ring )
{
  SPACE space = BufferSpace(buff, size);
  RingBufferReadBuffer(&space, ring);
  DATA data = BufferData(buff, &space);
  return data.Length;
}

void RingBufferReadBuffer(SPACE * space, struct RING_BUFFER * ring)
{
  while (! BufferFull(space) && ! RingBufferIsEmpty(ring)) {
    RingBufferReadSmall(space, ring);
  }
}

//TODO IMPLEMENT THIS IN TERMS OF THE BUFFER IMPL.
COUNT RingBufferWrite( char * buff, COUNT size, struct RING_BUFFER * ring )
{
  COUNT write = 0;
  while (write < size && !RingBufferIsFull( ring )) {
    write += RingBufferWriteSmall( buff+write, size-write, ring );
    ring->Empty = FALSE;
  }
  return write;
}

//TODO MAKE THIS THE BASE IMPL.
void RingBufferWriteBuffer(DATA * buff, struct RING_BUFFER * ring)
{
  COUNT write = RingBufferWrite(buff->Buff, buff->Length, ring);
  buff->Buff += write;
  buff->Length -= write;
}

BOOL RingBufferIsEmpty( struct RING_BUFFER * ring )
{
  return ring->Empty;
}

BOOL RingBufferIsFull( struct RING_BUFFER * ring )
{
  return ! ring->Empty && ring->WriteIndex == ring->ReadIndex;
}

void RingBufferInit( char * buff, COUNT size, struct RING_BUFFER * ring )
{
  ring->Buffer = buff;
  ring->Size = size;
  ring->ReadIndex=0;
  ring->WriteIndex=0;
  ring->Empty = TRUE;
}
