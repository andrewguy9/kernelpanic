#include<stdlib.h>

#include"ringbuffer.h"
#include"utils.h"

#define NUM_TESTS 4

COUNT TEST_SIZE;
COUNT RING_SIZE; 

struct RING_BUFFER Ring;
char * Buffer;
char * In;
char * Out;

void InitBuffers();
void PrintRing( struct RING_BUFFER * ring );
int Test();


void InitBuffers()
{
	int index;
	TEST_SIZE = rand()%128;
	RING_SIZE = 1 << (rand()%5+1);

	printf("Test on buffer size %d, ring size %d\n", 
			TEST_SIZE, 
			RING_SIZE );

	if( Buffer != NULL )
		free( Buffer );
	Buffer = malloc( RING_SIZE );

	if( In != NULL )
		free( In );
	In = malloc( TEST_SIZE );

	if( Out != NULL )
		free( Out );
	Out = malloc( TEST_SIZE );


	RingBufferInit(Buffer, RING_SIZE, &Ring);
	PrintRing( &Ring );

	printf("Initing buffers\n");
	for( index=0; index<TEST_SIZE; index++ )
	{
		In[index] = 'a'+rand()%26;
		Out[index] = 'x';
	}
}

void PrintRing( struct RING_BUFFER * ring )
{
	INDEX cur;
	INDEX read = ring->ReadIndex & ring->SizeMask;
	INDEX write = ring->WriteIndex & ring->SizeMask;

	if( RingBufferIsEmpty(ring))
	{
		printf("E");
	}
	else if( RingBufferIsFull(ring))
	{
		printf("F");
	}
	else
	{
		printf(" ");
	}

	printf("[");
	for( cur=0; cur < RING_SIZE; cur++ )
	{
		if( ring->WriteIndex % ring->Size == cur &&
				ring->ReadIndex % ring->Size == cur )
		{
			printf("b");
		}
		else if( ring->WriteIndex % ring->Size == cur )
		{
			printf("w");
		}
		else if( ring->ReadIndex % ring->Size == cur )
		{
			printf("r");
		}
		else
		{
			if( RingBufferIsEmpty(ring) )
			{//nothing in buffer.
				printf("-");
			}
			else if( ring->ReadIndex < ring->WriteIndex )
			{//data between is +
				if( cur > ring->ReadIndex && cur < ring->WriteIndex )
					printf("+");
				else
					printf("-");
			}
			else
			{//wrap around end.
				if( cur > ring->ReadIndex || cur < ring->WriteIndex )
					printf("+");
				else
					printf("-");
			}
		}
	}
	printf("]\n");
}

int Test()
{
	//populate and drain.
	INDEX index;
	INDEX read=0;
	INDEX write=0;
	INDEX delta=0;
	INDEX writePart;
	INDEX readPart;

	while( read < TEST_SIZE || write < TEST_SIZE )
	{
		writePart = rand()%(RING_SIZE+1);
		readPart = rand()%(RING_SIZE+1);

		if( write < TEST_SIZE )
		{
			delta = RingBufferWrite( In+write, MIN(writePart,TEST_SIZE-write), &Ring );
			write += delta;
			printf("write(%2d) returned %2d total %2d\t", 
					writePart, 
					delta, 
					write);
			PrintRing( &Ring );
			ASSERT(delta <= writePart, 0, "Wrote past buffer");
		}
		if( read < TEST_SIZE )
		{
			delta = RingBufferRead( Out+read, MIN(readPart, TEST_SIZE-read), &Ring );
			read += delta;
			printf("read (%2d) returned %2d total %2d\t", 
					readPart,
					delta, 
					read);
			PrintRing( &Ring );
			ASSERT( delta <= readPart, 0, "Read past buffer");
		}
	}

	ASSERT( read == TEST_SIZE, 0, "read wrong size");
	ASSERT( write == TEST_SIZE,0, "Write wrong size");

	//verify the buffer
	for( index = 0; index < TEST_SIZE; index++ )
	{
		if( In[index] != Out[index] )
		{
			printf("Failed index %d : in=%d out=%d\n", 
					index, 
					In[index], 
					Out[index]);
			return 1;
		}
	}
	printf("test passed\n");
	return 0;
}

int main()
{
	int testNum;

	for( testNum = 0; testNum < NUM_TESTS; testNum++ )
	{
		printf("---------------------------------------------------------------\n");
		//Initialize in and out buffers.
		InitBuffers();
		if( Test() )
			return 1;
		printf("passed\n");
	}
	printf("test round passed\n");
	return 0;
}
