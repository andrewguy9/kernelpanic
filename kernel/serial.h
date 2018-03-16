#ifndef SERIAL_H
#define SERIAL_H

#include"utils/types.h"
#include"utils/buffer.h"

void SerialStartup();
COUNT SerialWrite(char * buf, COUNT len);
void SerialSafeStrWrite(struct BUFF_CURSOR * str, struct BUFF_CURSOR * remstr);
//TODO Do we need a flush?
COUNT SerialRead(char * buf, COUNT len);

#endif
