#ifndef STR_H
#define STR_H

#include"utils/types.h"
#include"utils/utils.h"

struct SAFE_STR {
  char * Buff;
  COUNT Size;
};

struct SAFE_STR SafeStrInit(char * buff, COUNT size);
BOOL SafeStrFull(struct SAFE_STR * str);
#ifdef PC_BUILD
struct SAFE_STR SafeRender(struct SAFE_STR * str, char * format, ...);
#endif //PC_BUILD
COUNT SafeStrLen(struct SAFE_STR * basestr, struct SAFE_STR * remstr);

#endif //STR_H
