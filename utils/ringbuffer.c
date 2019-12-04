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
    struct RING_BUFFER * ring) {
  INDEX cur = ring->ReadIndex;
  INDEX end = ring->ReadIndex + size;
  //find where we stop
  if( ring->ReadIndex < ring->WriteIndex ) {
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
  //do copy
  INDEX index = 0;
  for (; cur != end; cur++) {
    buff[index++] = ring->Buffer[cur];
  }
  return index;
}

COUNT RingBufferWriteSmall( char *buff, COUNT size, struct RING_BUFFER * ring ) {
  INDEX cur = ring->WriteIndex;
  INDEX end = ring->WriteIndex + size;
  //find where we stop
  if (ring->WriteIndex < ring->ReadIndex) {
    //We can write to the read index only
    //[     w-----------r???e  ]
    if (end > ring->ReadIndex) {
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
  //do copy
  INDEX index = 0;
  for (; cur != end; cur++) {
    ring->Buffer[cur] = buff[index++];
  }
  return index;
}

_Bool RingBufferPeak( char * out, struct RING_BUFFER * ring) {
  if (RingBufferIsEmpty(ring)) {
    return false;
  } else {
    *out = ring->Buffer[ring->ReadIndex];
    return true;
  }
}

//
//Public routines
//
COUNT RingBufferRead( char * buff, COUNT size, struct RING_BUFFER * ring ) {
  COUNT read=0;
  while (read < size && ! RingBufferIsEmpty( ring )) {
    read += RingBufferReadSmall( buff+read, size-read, ring );
    if (ring->ReadIndex == ring->WriteIndex) {
      ring->Empty = true;
    }
  }
  return read;
}

COUNT RingBufferWrite( char * buff, COUNT size, struct RING_BUFFER * ring ) {
  COUNT write = 0;
  while (write < size && !RingBufferIsFull( ring )) {
    write += RingBufferWriteSmall( buff+write, size-write, ring );
    ring->Empty = false;
  }
  return write;
}

_Bool RingBufferIsEmpty( struct RING_BUFFER * ring ) {
  return ring->Empty;
}

_Bool RingBufferIsFull( struct RING_BUFFER * ring ) {
  return ! ring->Empty && ring->WriteIndex == ring->ReadIndex;
}

void RingBufferInit( char * buff, COUNT size, struct RING_BUFFER * ring ) {
  ring->Buffer = buff;
  ring->Size = size;
  ring->ReadIndex=0;
  ring->WriteIndex=0;
  ring->Empty = true;
}
