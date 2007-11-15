#include"ringbuffer.h"

//
//Private Routines
//

COUNT RingBufferReadSmall(char *buff, COUNT size, struct RING_BUFFER * ring )
{
	INDEX cur;
	INDEX end = ring->ReadIndex+size;
	//find where we stop
	if( ring->ReadIndex < ring->WriteIndex )
	{//We can read to write index only
		if( end > ring->WriteIndex )
		{
			end = ring->WriteIndex;	
		}
		//Update the ReadIndex to its future position.
		ring->ReadIndex = end;
	}
	else
	{//We can read to end of buffer
		if( end > ring->Size )
		{
			end = ring->Size;
			//Wrap the ReadIndex back to 0;
			ring->ReadIndex = 0;
		}
		else
		{
			//Read isn't to end, so dont wrap.
			ring->ReadIndex = end;
		}
	}
	//do copy
	INDEX index;
	while( cur = ring->ReadIndex; cur != end; cur++ )
	{
		buff[index++] = ring->Buffer[cur];
	}
	return index;
}

//
//Public routines
//
COUNT RingBufferRead( char * buff, COUNT size, struct RING_BUFFER * ring )
{
	COUNT read=0;
	while( read < size && ! RingBufferIsEmpty( ring ) )
	{
		read += RingBufferSmallRead( buff+read, size-read, ring );
	}
	if( ring->ReadIndex == ring->WriteIndex )
	{
		ring->Empty = TRUE;
	}
	return read;
}

COUNT RingBufferWrite( char * buff, COUNT size, struct RING_BUFFER * ring )
{
	COUNT write = 0;
	while( write < size && !RingBufferIsFull( ring ) )
	{
		write += RingBufferWriteSmall( buff+write, size-write, ring );
	}
	ring->Empty = FALSE;
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
