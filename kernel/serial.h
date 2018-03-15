#ifndef SERIAL_H
#define SERIAL_H

#include"utils/types.h"
#include"utils/str.h"

void SerialStartup();
COUNT SerialWrite(char * buf, COUNT len);
void SerialSafeStrWrite(struct SAFE_STR * str, struct SAFE_STR * remstr);
//TODO Do we need a flush?
COUNT SerialRead(char * buf, COUNT len);

#endif
