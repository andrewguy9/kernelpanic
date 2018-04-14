#ifndef SERIAL_H
#define SERIAL_H

#include"utils/types.h"
#include"utils/buffer.h"

void SerialStartup();
void SerialWriteBuffer(DATA * str);
//TODO Do we need a flush?
void SerialReadBuffer(SPACE * s);

#endif
