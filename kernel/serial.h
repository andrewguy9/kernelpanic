#ifndef SERIAL_H
#define SERIAL_H

#include"utils/types.h"
#include"utils/buffer.h"

void SerialStartup();
COUNT SerialWrite(char * buf, COUNT len);
void SerialWriteBuffer(DATA * str);
//TODO Do we need a flush?
COUNT SerialRead(char * buf, COUNT len);
void SerialReadBuffer(SPACE * s);

#endif
