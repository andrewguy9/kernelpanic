#include"ringbuffer.h"

//
//Private Routines
//

COUNT RingBufferReadSmall( char *buff, COUNT buffSize, char * ring, COUNT ringSize, COUNT readIndex, COUNT writeIndex )
{
	INDEX cur = readIndex;
	INDEX end = readIndex + buffSize;
	//find where we stop
	if( readIndex < writeIndex )
	{
		//We can read up to write index only 
		//[     r-----------w???e  ]
		if( end > writeIndex )
		{
			//Make sure we don't go past WriteIndex
			//[     r----------ew     ]
			end = writeIndex;	
		}
	}
	else
	{
		//We can read to end of buffer
		//[     w     r-----------]???e
		if( end >= ringSize )
		{
			//Make sure we don't read past end of buffer
			//[     w     r----------e]
			end = ringSize;
		}
	}
	//do copy
	INDEX index = 0;
	for( ; cur != end; cur++ )
	{
		buff[index++] = ring[cur];
	}
	return index;
}

COUNT RingBufferWriteSmall( char *buff, COUNT buffSize, char * ring, COUNT ringSize, COUNT readIndex, COUNT writeIndex )
{
	INDEX cur = writeIndex;
	INDEX end = writeIndex + buffSize;
	//find where we stop
	if( writeIndex < readIndex)
	{
		//We can write to the read index only
		//[     w-----------r???e  ]
		if( end > readIndex )
		{
			//Make sure we don't go past ReadIndex
			//[     w----------er     ]
			end = readIndex-1;
		}
	}
	else
	{
		//We can write to end of buffer
		//[     r     w-----------]???e
		if( end >= ringSize )
		{
			//Make sure we don't write past end of buffer
			//[     r     w----------e]
			end = ringSize;
		}
	}
	//do copy
	INDEX index = 0;
	for( ; cur != end; cur++ )
	{
		ring[cur] = buff[index++];
	}
	return index;
}	

//
//Public routines
//
COUNT RingBufferRead( char * buff, COUNT size, struct RING_BUFFER * ring )
{
	COUNT read=0;
	COUNT delta;
	while( read < size && ! RingBufferIsEmpty( ring ) )
	{
		delta = RingBufferReadSmall( 
				buff+read, 
				size-read, 
				ring->Buffer, 
				ring->Size, 
				ring->ReadIndex & ring->SizeMask, 
				ring->WriteIndex & ring->SizeMask );
		read += delta;
		ring->ReadIndex += delta;
	}
	return read;
}

COUNT RingBufferWrite( char * buff, COUNT size, struct RING_BUFFER * ring )
{
	COUNT write = 0;
	COUNT delta;
	while( write < size && ! RingBufferIsFull( ring ) )
	{
		delta = RingBufferWriteSmall( 
				buff+write, 
				size-write, 
				ring->Buffer,
				ring->Size,
				ring->ReadIndex & ring->SizeMask,
				ring->WriteIndex & ring->SizeMask);
		write += delta;
		ring->WriteIndex+=delta;
	}
	return write;
}

BOOL RingBufferIsEmpty( struct RING_BUFFER * ring )
{
	return ring->ReadIndex == ring->WriteIndex;
}

BOOL RingBufferIsFull( struct RING_BUFFER * ring )
{
	return (ring->WriteIndex - ring->ReadIndex) +1 >= ring->Size;
}

void RingBufferInit( char * buff, COUNT size, struct RING_BUFFER * ring )
{
	int bit;

	ring->Buffer = buff;
	ring->Size = size;
	ring->ReadIndex=0;
	ring->WriteIndex=0;

	ring->SizeMask = 0;//load ffff into SizeMask.
	for( bit=0; bit < sizeof( COUNT ) * 8; bit++ )
	{
		if( 1 << bit >= ring->Size )
			ring->SizeMask |= 1 << bit;//turn on bit if greater than or equal to size.
	}
		ASSERT(  ( ring->Size & ring->SizeMask ) == 0,
			RING_BUFFER_INIT_INVALID_SIZE,
			"Size must be a power of 2");

	//Invert the size mask to put zeros in high part.
	ring->SizeMask = ~ ring->SizeMask;


	ASSERT( ring->Size < 1 << ( sizeof(COUNT) * 8 -1 ),
			RING_BUFFER_INIT_SIZE_TOO_BIG,
			"Ring buffer must have 0 for MSB");
}
