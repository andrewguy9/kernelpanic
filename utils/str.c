#include"utils/str.h"

#include<stdio.h>
#include <stdarg.h>

/*
 * Makes a buffer which is safe to render into or write to serial
 * First byte is EOS sperator. Size is usable space left.
 */
struct SAFE_STR SafeStrInit(char * buff, COUNT size)
{
  ASSERT (size > 0);
  struct SAFE_STR str = {buff, size-1};
  buff[0] = '\0';
  return str;
}

BOOL SafeStrFull(struct SAFE_STR * str)
{
  return str->Size == 0;
}

#ifdef PC_BUILD
/*
 * Render a string into str.
 * Returns remaning string.
 * Buf will point at null terminator.
 * Size will be usable space left.
 */
struct SAFE_STR SafeRender(struct SAFE_STR * str, char * format, ...)
{
  int status;
  va_list args;
  va_start (args, format);
  status = vsnprintf(str->Buff, str->Size, format, args);
  if (status >= str->Size) {
    //TODO Write was truncated.
    struct SAFE_STR full = {str->Buff + str->Size};
    return full;
  } else if (status < 0) {
    //TODO capture errno.
    KernelPanic();
  } else {
    struct SAFE_STR output = {str->Buff + status, str->Size - status};
    return output;
  }
}
#endif //PC_BUILD

COUNT SafeStrLen(struct SAFE_STR * basestr, struct SAFE_STR * remstr) {
  COUNT written = remstr->Buff - basestr->Buff;
  ASSERT (written < basestr->Size);
  return written;
}

