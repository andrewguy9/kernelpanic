#include"ringbuffer.h"
#include"utils.h"

#define TEST_SIZE 1024
#define RING_SIZE 32
#define WRITE_PART 14
#define READ_PART 11

struct RING_BUFFER Ring;
char Buffer[RING_SIZE];
char In[TEST_SIZE];
char Out[TEST_SIZE];

int main()
{
	RingBufferInit(Buffer, RING_SIZE, &Ring);

	//Initialize in and out buffers.
	printf("Initing buffers\n");
	INDEX index;
	for( index=0; index<TEST_SIZE; index++ )
	{
		In[index] = index;
		Out[index] = index;
	}

	//populate and drain.
	INDEX read=0;
	INDEX write=0;
	INDEX delta=0;
	while( read < TEST_SIZE || write < TEST_SIZE )
	{
		if( write < TEST_SIZE )
		{
			delta= RingBufferWrite( In+write, WRITE_PART, &Ring );
			write += delta;
			//printf("write %d returned %d\n", WRITE_PART, delta);
			ASSERT(delta <= WRITE_PART, 0, "Wrote past buffer");
		}
		if( read < TEST_SIZE )
		{
			delta = RingBufferRead( Out+read, READ_PART, &Ring );
			read += delta;
			//printf("read %d returned %d\n", READ_PART, delta);
			ASSERT( delta <= READ_PART, 0, "Read past buffer");
		}
	}

	ASSERT( read == TEST_SIZE, 0, "read wrong size");
	ASSERT( write == TEST_SIZE,0, "Write wrong size");

	//verify the buffer
	for( index = 0; index< TEST_SIZE; index++ )
	{
		if( In[index] != Out[index] )
		{
			printf("Failed index %d : in=%d out=%d\n", index, );
			return 1;
		}
	}
	return 0;
}
