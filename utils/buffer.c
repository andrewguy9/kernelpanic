#include"utils/buffer.h"

#include<stdio.h>
#include <stdarg.h>

/*
 * Makes a buffer which is safe to render into or write to serial
 * First byte is EOS sperator. Size is usable space left.
 */
struct BUFF_CURSOR BuffCursorInit(char * buff, COUNT size)
{
  ASSERT (size > 0);
  struct BUFF_CURSOR buf = {buff, size-1};
  buff[0] = '\0';
  return buf;
}

BOOL BuffFull(struct BUFF_CURSOR * buf)
{
  return buf->Size == 0;
}

#ifdef PC_BUILD
/*
 * Render a string into buffer.
 * Returns remaning buffer.
 * Buf will point at null terminator.
 * Size will be usable space left.
 */
struct BUFF_CURSOR BuffStrRender(struct BUFF_CURSOR * buf, char * format, ...)
{
  int status;
  va_list args;
  va_start (args, format);
  status = vsnprintf(buf->Buff, buf->Size, format, args);
  if (status >= buf->Size) {
    //TODO Write was truncated.
    struct BUFF_CURSOR full = {buf->Buff + buf->Size};
    return full;
  } else if (status < 0) {
    //TODO capture errno.
    FAIL("unhandled error from vsnprintf");
  } else {
    struct BUFF_CURSOR output = {buf->Buff + status, buf->Size - status};
    return output;
  }
}
#endif //PC_BUILD

COUNT BuffSpace(struct BUFF_CURSOR * base, struct BUFF_CURSOR * rem) {
  COUNT written = rem->Buff - base->Buff;
  ASSERT (written < base->Size);
  return written;
}

