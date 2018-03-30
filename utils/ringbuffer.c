#include"ringbuffer.h"

/*
 * Implements a ring buffer.
 */

//
//Private Routines
//

COUNT RingBufferReadSmall(
    char *buff,
    COUNT size,
    struct RING_BUFFER * ring)
{
  INDEX start = ring->ReadIndex;
  INDEX end = ring->ReadIndex + size;
  //find where we stop
  if (ring->ReadIndex < ring->WriteIndex)
  {
    //We can read up to write index only 
    //[     r-----------w???e  ]
    if (end > ring->WriteIndex) {
      //Make sure we don't go past WriteIndex
      //[     r----------ew     ]
      end = ring->WriteIndex;
    }
    //Update the ReadIndex to its future position.
    //[     c++++++er----w     ]
    ring->ReadIndex = end;
  } else {
    //We can read to end of buffer
    //[     w     r-----------]???e
    if (end >= ring->Size) {
      //Make sure we don't read past end of buffer
      //[     w     r----------e]
      end = ring->Size;
      //Wrap the ReadIndex back to 0;
      //[r    w     c----------e]
      ring->ReadIndex = 0;
    } else {
      //Read isn't to end, so dont wrap.
      //[     w     r-------e---]
      ring->ReadIndex = end;
    }
  }
  COUNT len = end-start;
  DATA data = BufferSpace(ring->Buffer+start, len);
  SPACE space = BufferSpace(buff, len);
  BufferCopy(&data, &space);
  return end-start;
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
  COUNT read=0;
  while (read < size && ! RingBufferIsEmpty( ring )) {
    read += RingBufferReadSmall( buff+read, size-read, ring );
    if (ring->ReadIndex == ring->WriteIndex) {
      ring->Empty = TRUE;
    }
  }
  return read;
}

void RingBufferReadBuffer(SPACE * buff, struct RING_BUFFER * ring)
{
  //TODO WE DO POINTER MATH HERE, BUT I'M OK WITH IT.
  COUNT read = RingBufferRead(buff->Buff, buff->Length, ring);
  buff->Buff += read;
  buff->Length -= read;
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
