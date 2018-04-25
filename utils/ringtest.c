#include<stdlib.h>
#include<stdio.h>

#include"ringbuffer.h"
#include"buffer.h"
#include"utils.h"

#define NUM_TESTS 4

/*
 * Tests ringbuffer unit. Tries a variety of passes on the buffer.
 * Prints success or failure for validation.
 */

COUNT TEST_SIZE;
COUNT RING_SIZE;

struct RING_BUFFER Ring;
char * RingBuffer;
char * InBuffer;
DATA In;
char * OutBuffer;
SPACE Out;

void InitBuffers();
void PrintRing( struct RING_BUFFER * ring );
int Test();


void InitBuffers() {
  TEST_SIZE = rand()%128;
  RING_SIZE = rand()%32;

  printf("Test on buffer size %ld, ring size %ld\n",
      TEST_SIZE,
      RING_SIZE);

  if (RingBuffer != NULL) {
    free( RingBuffer );
  }
  RingBuffer = malloc(RING_SIZE);

  if (InBuffer != NULL) {
    free(InBuffer);
  }
  InBuffer = malloc(TEST_SIZE);
  SPACE inSpace = BufferSpace(InBuffer, TEST_SIZE);

  if (OutBuffer != NULL) {
    free(OutBuffer);
  }
  OutBuffer = malloc(TEST_SIZE);
  SPACE outSpace = BufferSpace(OutBuffer, TEST_SIZE);

  SPACE space = BufferSpace(RingBuffer, RING_SIZE);
  RingBufferInit(&space, &Ring);
  PrintRing( &Ring );

  printf("Initing buffers\n");
  for (int index=0; index<TEST_SIZE; index++) {
    BufferPrint(&inSpace, "%c", 'a'+rand()%26);
    BufferPrint(&outSpace, "x");
  }
  In = BufferData(InBuffer, &inSpace);
  Out = BufferSpace(OutBuffer, TEST_SIZE);
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
  while (!BufferFull(&Out) || !BufferEmpty(&In)) {
    if (!BufferEmpty(&In)) {
      printf("Writing\n");
      RingBufferWriteBuffer(&In, &Ring);
      PrintRing( &Ring );
    }
    if (!BufferFull(&Out)) {
      printf("Reading\n");
      RingBufferReadBuffer(&Out, &Ring);
      PrintRing( &Ring );
    }
  }

  printf("%s\n", InBuffer);
  printf("%s\n", OutBuffer);
  //verify the buffer
  DATA out = BufferData(OutBuffer, &Out);
  DATA in = BufferData(InBuffer, &In);
  if (! BufferCompare(&in, &out)) {
    printf("Buffer mismatch!\n");
    return 1;
  } else {
    printf("test passed\n");
    return 0;
  }
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
