#ifndef BUFF_H
#define BUFF_H

#include"utils/types.h"
#include"utils/utils.h"

struct BUFF_CURSOR {
  char * Buff;
  COUNT Size;
};

struct BUFF_CURSOR BuffCursorInit(char * buff, COUNT size);
BOOL BuffFull(struct BUFF_CURSOR * str);
#ifdef PC_BUILD
struct BUFF_CURSOR BuffStrRender(struct BUFF_CURSOR * buf, char * format, ...);
#endif //PC_BUILD
COUNT BuffSpace(struct BUFF_CURSOR * base, struct BUFF_CURSOR * rem);

#endif //BUFF_H
