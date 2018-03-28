#include"utils/buffer.h"
#include"utils/utils.h"

#include<stdio.h>
#include <stdarg.h>

/*
 * Makes a buffer which is safe to render into or write to serial
 * First byte is EOS sperator. Length is usable space left.
 */
SPACE BufferSpace(char * buff, COUNT size)
{
  SPACE space = {buff, size};
  return space;
}

#ifdef PC_BUILD
#include<string.h>
BOOL BufferCopy(DATA * d, SPACE * s)
{
  if (d->Length > s->Length) {
    return FALSE;
  } else {
    memcpy(d->Buff, s->Buff, d->Length);
    s->Length -= d->Length;
    s->Buff += d->Length;
    return TRUE;
  }
}
#endif // PC_BUILD

DATA BufferData(char * buff, SPACE * s)
{
  DATA d = {buff, s->Buff - buff};
  return d;
}

BOOL BufferFull(SPACE * s)
{
  return s->Length == 0;
}

BOOL BufferEmpty(DATA * d)
{
  return d->Length == 0;
}

#ifdef PC_BUILD
/*
 * Render a string into buffer.
 * Returns remaning buffer.
 * Buf will point at null terminator.
 * Length will be usable space left.
 */
BOOL BufferPrint(SPACE * s, char * format, ...)
{
  int status;
  va_list args;
  va_start (args, format);
  status = vsnprintf(s->Buff, s->Length, format, args);
  if (status >= s->Length) {
    s->Buff = s->Buff + s->Length;
    s->Length = 0;
    return FALSE;
  } else if (status < 0) {
    //TODO capture errno.
    FAIL("unhandled error from vsnprintf");
  } else {
    s->Buff += status;
    s->Length -= status;
    return TRUE;
  }
}
#endif //PC_BUILD
