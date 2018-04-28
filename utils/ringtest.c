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


void PrintRing( struct RING_BUFFER * ring );

DATA InitIn(COUNT testSize) {
  char * InBuffer = malloc(testSize);
  SPACE inSpace = BufferSpace(InBuffer, testSize);
  printf("Initing input\n");
#if 0
  for (int index=0; index<testSize; index++) {
    //TODO HANDLE ERROR FROM PRINT.
    BufferPrint(&inSpace, "%c", 'a'+rand()%26);
  }
#elif 1
  while(!BufferFull(&inSpace)) {
    BufferPrint(&inSpace, "%c", 'a'+rand()%26);
  }
#else
  //TODO MAKE A MACRO
#endif
  DATA In = BufferData(InBuffer, &inSpace);
  return In;
}

SPACE InitOut(COUNT testSize) {
  char * OutBuffer = malloc(testSize);
  SPACE outSpace = BufferSpace(OutBuffer, testSize);
  printf("Initing output\n");
  //TODO USE ABOVE MACRO.
  for (int index=0; index<testSize; index++) {
    BufferPrint(&outSpace, "*");
  }
  SPACE out = BufferSpace(OutBuffer, testSize);
  return out;
}

struct RING_BUFFER InitRing() {
  COUNT ringSize = rand()%32;
  printf("Test on ring size %ld\n", ringSize);
  char * RingBuffer;
  RingBuffer = malloc(ringSize);
  SPACE space = BufferSpace(RingBuffer, ringSize);
  struct RING_BUFFER Ring;
  RingBufferInit(&space, &Ring);
  PrintRing( &Ring );
  return Ring;
}

void FreeBuffer(struct BUFFER * buff) {
  free(buff->Buff); //XXX CRACKING BUFFERS
  *buff = BufferNull;
}

void FreeRing(struct RING_BUFFER * ring) {
  free(ring->Buffer);
  ring->Buffer = NULL;
}

void PrintRing( struct RING_BUFFER * ring ) {
  if (ring->Empty) {
    printf("E");
  } else {
    printf(" ");
  }
  printf("[");
  //TODO use for each macro.
  for (INDEX cur=0; cur < ring->Size; cur++) {
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

void MoveData(DATA in, SPACE out, struct RING_BUFFER ring) {
  while (!BufferFull(&out) || !BufferEmpty(&in)) {
    if (!BufferEmpty(&in)) {
      printf("Writing\n");
      RingBufferWriteBuffer(&in, &ring);
      PrintRing( &ring );
    }
    if (!BufferFull(&out)) {
      printf("Reading\n");
      RingBufferReadBuffer(&out, &ring);
      PrintRing( &ring );
    }
  }
}

int Test(DATA in, SPACE out, struct RING_BUFFER ring) {
  MoveData(in, out, ring);
  printf("%s\n", in.Buff); //XXX CRACKING BUFFER
  printf("%s\n", out.Buff); //XXX CRACKING BUFFER
  //verify the buffer
  if (! BufferCompare(&in, &out)) {
    printf("Buffer mismatch!\n");
    return 1;
  } else {
    printf("test passed\n");
    return 0;
  }
}

int main() {
  for (int testNum = 0; testNum < NUM_TESTS; testNum++) {
    printf("---------------------------------------------------------------\n");
    //Initialize in and out buffers.
    COUNT testSize = rand()%128;
    printf("Test on buffer size %ld\n", testSize);
    DATA in = InitIn(testSize);
    SPACE out = InitOut(testSize);
    struct RING_BUFFER ring = InitRing();
    if (Test(in, out, ring)) {
      return 1;
    }
    FreeBuffer(&in);
    FreeBuffer(&out);
    FreeRing(&ring);
    printf("passed\n");
  }
  printf("test round passed\n");
  return 0;
}
