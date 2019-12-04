#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include"utils.h"

struct RING_BUFFER {
	char * Buffer;
	COUNT Size;
	INDEX ReadIndex;
	INDEX WriteIndex;
	_Bool Empty;
};

COUNT RingBufferRead( char * buff, COUNT size, struct RING_BUFFER * ring );
_Bool RingBufferPeak( char * out, struct RING_BUFFER * ring);
COUNT RingBufferWrite( char * buff, COUNT size, struct RING_BUFFER * ring );
_Bool RingBufferIsEmpty( struct RING_BUFFER * ring );
_Bool RingBufferIsFull( struct RING_BUFFER * ring );
void RingBufferInit( char * buff, COUNT size, struct RING_BUFFER * ring );

#endif
