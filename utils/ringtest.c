#include<stdlib.h>
#include<stdio.h>

#include"ringbuffer.h"
#include"utils.h"

#define NUM_TESTS 4

/*
 * Tests ringbuffer unit. Tries a variety of passes on the buffer.
 * Prints success or failure for validation.
 */

COUNT TEST_SIZE;
COUNT RING_SIZE;

struct RING_BUFFER Ring;
char * Buffer;
char * In;
char * Out;

void InitBuffers();
void PrintRing( struct RING_BUFFER * ring );
int Test();


void InitBuffers() {
  TEST_SIZE = rand()%128;
  RING_SIZE = rand()%32;

  printf("Test on buffer size %ld, ring size %ld\n",
      TEST_SIZE,
      RING_SIZE);

  if (Buffer != NULL) {
    free( Buffer );
  }
  Buffer = malloc(RING_SIZE);

  if (In != NULL) {
    free(In);
  }
  In = malloc(TEST_SIZE);

  if (Out != NULL) {
    free(Out);
  }
  Out = malloc(TEST_SIZE);

  RingBufferInit(Buffer, RING_SIZE, &Ring);
  PrintRing( &Ring );

  printf("Initing buffers\n");
  for( int index=0; index<TEST_SIZE; index++ ) {
    In[index] = 'a'+rand()%26;
    Out[index] = 'x';
  }
}

void PrintRing( struct RING_BUFFER * ring ) {
  if (ring->Empty) {
    printf("E");
  } else {
    printf(" ");
  }
  printf("[");
  for (INDEX cur=0; cur < RING_SIZE; cur++) {
    if( ring->WriteIndex == cur && ring->ReadIndex == cur ) {
      printf("b");
    } else if( ring->WriteIndex == cur ) {
      printf("w");
    } else if( ring->ReadIndex == cur ) {
      printf("r");
    } else {
      if( ring->Empty ) {
        //nothing in buffer.
        printf("-");
      } else if( ring->ReadIndex < ring->WriteIndex ) {
        //data between is +
        if( cur > ring->ReadIndex && cur < ring->WriteIndex ) {
          printf("+");
        } else {
          printf("-");
        }
      } else {
        //wrap around end.
        if( cur > ring->ReadIndex || cur < ring->WriteIndex ) {
          printf("+");
        } else {
          printf("-");
        }
      }
    }
  }
  printf("]\n");
}

int Test() {
  //populate and drain.
  INDEX index;
  INDEX read=0;
  INDEX write=0;
  INDEX delta=0;
  INDEX writePart;
  INDEX readPart;

  while (read < TEST_SIZE || write < TEST_SIZE) {
    writePart = (rand()%TEST_SIZE)+1;
    readPart = (rand()%TEST_SIZE)+1;

    if (write < TEST_SIZE) {
      delta = RingBufferWrite( In+write, MIN(writePart,TEST_SIZE-write), &Ring );
      write += delta;
      printf("write(%2ld) returned %2ld total %2ld\t",
          writePart,
          delta,
          write);
      PrintRing( &Ring );
      ASSERT(delta <= writePart);
    }
    if (read < TEST_SIZE) {
      delta = RingBufferRead( Out+read, MIN(readPart, TEST_SIZE-read), &Ring );
      read += delta;
      printf("read (%2ld) returned %2ld total %2ld\t",
          readPart,
          delta,
          read);
      PrintRing( &Ring );
      ASSERT( delta <= readPart);
    }
  }

  ASSERT( read == TEST_SIZE);
  ASSERT( write == TEST_SIZE);

  //verify the buffer
  for (index = 0; index < TEST_SIZE; index++) {
    if (In[index] != Out[index]) {
      printf("Failed index %ld : in=%c out=%c\n",
          index,
          In[index],
          Out[index]);
      return 1;
    }
  }
  printf("test passed\n");
  return 0;
}

int main() {
  int testNum;

  for (testNum = 0; testNum < NUM_TESTS; testNum++) {
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
