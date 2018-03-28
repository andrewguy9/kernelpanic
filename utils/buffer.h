#ifndef BUFF_H
#define BUFF_H

#include"utils/types.h"
#include"utils/utils.h"

struct BUFFER {
  char * Buff;
  COUNT Length;
};

// Used to represent full buffer sections.
typedef struct BUFFER DATA;
// Used to represent empty buffer sections
typedef struct BUFFER SPACE;

// Initialize empty buffer.
SPACE BufferSpace(char * buff, COUNT size);

#ifdef PC_BUILD
BOOL BufferCopy(DATA * d, SPACE * s);
#endif //PC_BUILD

#define BufferFromObj(obj) ((DATA) BufferSpace( (char*) (&(obj)), sizeof(obj)))
#define BufferToObj(data, type) (*(type *) data.Buff)

DATA BufferData(char * buff, SPACE * s);

BOOL BufferFull(SPACE * s);
BOOL BufferEmpty(DATA * d);

#ifdef PC_BUILD
BOOL BufferPrint(SPACE * s, char * format, ...);
#endif //PC_BUILD

#ifdef PC_BUILD
BOOL BufferCompare(const DATA * d1, const DATA * d2);
#endif //PC_BUILD
#endif //BUFF_H
