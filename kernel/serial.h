#ifndef SERIAL_H
#define SERIAL_H

#include"utils/types.h"

void SerialStartup();
COUNT SerialWrite(char * buf, COUNT len);
COUNT SerialRead(char * buf, COUNT len);

#endif
