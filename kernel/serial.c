#include"serial.h"
#include"hal.h"
#include"critinterrupt.h"
#include"utils/ringbuffer.h"
#include"generation.h"

#define BUFFER_SIZE 32

char SerialInputBuffer[BUFFER_SIZE];
char SerialOutputBuffer[BUFFER_SIZE];

struct RING_BUFFER SerialInputRing;
struct RING_BUFFER SerialOutputRing;

struct GENERATION ReadGeneration;
COUNT ReadGenerationCount;
struct HANDLER_OBJECT ReadGenerationCritObject;
struct GENERATION_CONTEXT ReadGenerationContext;

ISR_HANDLER SendBytesInterrupt;
ISR_HANDLER GetBytesInterrupt;

void SendBytesInterrupt(void)
{
  ASSERT(IsrIsAtomic(IRQ_LEVEL_SERIAL_WRITE));
  while (!RingBufferIsEmpty(&SerialOutputRing)) {
    char data;
    CHECK(RingBufferRead(&data, sizeof(data), &SerialOutputRing) == 1);
    HalSerialWriteChar(data);
  }
}

void GetBytesInterrupt(void)
{
  while (!RingBufferIsFull(&SerialInputRing)) {
    char data;
    if (HalSerialGetChar(&data)) {
      CHECK(RingBufferWrite(&data, sizeof(data), &SerialInputRing) == 1);
    } else {
      break;
    }
  }

  if ( HandlerIsFinished(&ReadGenerationCritObject) ) {
    GenerationUpdateSafe(
        &ReadGeneration,
        ++ReadGenerationCount,
        &ReadGenerationContext,
        &ReadGenerationCritObject);
  }
}

void SerialStartup()
{
  RingBufferInit( SerialInputBuffer, BUFFER_SIZE, &SerialInputRing );
  RingBufferInit( SerialOutputBuffer, BUFFER_SIZE, &SerialOutputRing );

  ReadGenerationCount=0;
  GenerationInit(&ReadGeneration, ReadGenerationCount);
  HandlerInit(&ReadGenerationCritObject);

  IsrRegisterHandler( SendBytesInterrupt, (void *) HAL_ISR_SERIAL_WRITE, IRQ_LEVEL_SERIAL_WRITE);
  IsrRegisterHandler( GetBytesInterrupt, (void *) HAL_ISR_SERIAL_READ, IRQ_LEVEL_SERIAL_READ);

  HalStartSerial();
}

COUNT SerialWrite(char * buf, COUNT len)
{
  COUNT write;

  IsrDisable(IRQ_LEVEL_SERIAL_WRITE);
  write = RingBufferWrite(buf, len, &SerialOutputRing);
  IsrEnable(IRQ_LEVEL_SERIAL_WRITE);

  HalRaiseInterrupt(IRQ_LEVEL_SERIAL_WRITE);

  return write;
}

COUNT SerialRead(char * buf, COUNT len)
{
  COUNT read = 0;
  COUNT readGeneration;
  _Bool wasFull = false;

  do {
    IsrDisable(IRQ_LEVEL_SERIAL_READ);

    if ( RingBufferIsFull( &SerialInputRing ) ) {
      wasFull = true;
    }

    read = RingBufferRead(buf, len, &SerialInputRing);
    readGeneration = ReadGenerationCount;

    IsrEnable(IRQ_LEVEL_SERIAL_READ);

    if (read == 0) {
      GenerationWait(&ReadGeneration, readGeneration, NULL);
    }
  } while (read == 0);

  // We were full, so lets make sure there wasn't any data buffered
  // in the hal.
  if ( wasFull ) {
    HalRaiseInterrupt( IRQ_LEVEL_SERIAL_READ );
  }
  return read;
}

_Bool SerialPeak(char * out) {
  IsrDisable(IRQ_LEVEL_SERIAL_READ);
  _Bool result = RingBufferPeak(out, &SerialInputRing);
  IsrEnable(IRQ_LEVEL_SERIAL_READ);
  return result;
}
