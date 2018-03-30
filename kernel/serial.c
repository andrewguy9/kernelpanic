#include"serial.h"
#include"hal.h"
#include"critinterrupt.h"
#include"utils/ringbuffer.h"
#include"utils/buffer.h"
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

char SendBytesBufferFull;
char SendBytesBuffer;
void SendBytesInterrupt(void)
{
  ASSERT(IsrIsAtomic(IRQ_LEVEL_SERIAL_WRITE));
  while (!RingBufferIsEmpty(&SerialOutputRing)) {
    if (!SendBytesBufferFull) {
      ASSUME(RingBufferRead(&SendBytesBuffer, sizeof(SendBytesBuffer), &SerialOutputRing), 1);
      SendBytesBufferFull = TRUE;
    }
    SendBytesBufferFull = !HalSerialWriteChar(SendBytesBuffer);
    if (SendBytesBufferFull) {
      break; //Hal is putting back pressure on us.
    }
  }
}

void GetBytesInterrupt(void)
{
  while (!RingBufferIsFull(&SerialInputRing)) {
    char data;
    if (HalSerialGetChar(&data)) {
      ASSUME(RingBufferWrite(&data, sizeof(data), &SerialInputRing), 1);
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
  SendBytesBufferFull = FALSE;
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
  DATA data = BufferSpace(buf, len);
  SerialWriteBuffer(&data);
  //XXX NOTE THAT data ALWAYS HAS LENGTH len
  return data.Length;
}

void SerialWriteBuffer(DATA * data) {
  while (! BufferEmpty(data)) {
    IsrDisable(IRQ_LEVEL_SERIAL_WRITE);
    RingBufferWriteBuffer(data, &SerialOutputRing);
    IsrEnable(IRQ_LEVEL_SERIAL_WRITE);

    HalRaiseInterrupt(IRQ_LEVEL_SERIAL_WRITE);
  }
}

COUNT SerialRead(char * buf, COUNT len)
{
  SPACE space = BufferSpace(buf, len);
  SerialReadBuffer(&space);
  DATA data = BufferData(buf, &space);
  return data.Length;
}

void SerialReadBuffer(SPACE * space)
{
  char * startBuff = space->Buff;
  DATA readData;
  do {
    IsrDisable(IRQ_LEVEL_SERIAL_READ);

    BOOL wasFull = RingBufferIsFull(&SerialInputRing);

    RingBufferReadBuffer(space, &SerialInputRing);
    COUNT readGeneration = ReadGenerationCount;

    IsrEnable(IRQ_LEVEL_SERIAL_READ);

    readData = BufferData(startBuff, space);
    //TODO NEED A GOOD WAY TO CALCULATE HOW MUCH WAS READ.
    if (BufferEmpty(&readData)) {
      GenerationWait(&ReadGeneration, readGeneration, NULL);
    }
    if ( wasFull ) {
      // We were full, so lets make sure there wasn't any data buffered
      // in the hal.
      HalRaiseInterrupt( IRQ_LEVEL_SERIAL_READ );
    }
  } while (BufferEmpty(&readData));
}
