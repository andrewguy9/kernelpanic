#include"utils/buffer.h"
#include"utils/utils.h"

#include<stdio.h>
#include <stdarg.h>

#ifdef PC_BUILD
#include<string.h>
//TODO All non char oriented buffer copies are unsafe.
//We might break reads across objects.
void BufferCopy(DATA * data, SPACE * space)
{
  COUNT len = MIN(data->Length, space->Length);
  memcpy(space->Buff, data->Buff, len);
  space->Length -= len;
  space->Buff += len;
  data->Length -= len;
  data->Buff += len;
}
#endif // PC_BUILD

const struct BUFFER BufferNull = {NULL, 0};

DATA BufferData(char * buff, const SPACE * s)
{
  DATA d = {buff, s->Buff - buff};
  return d;
}

BOOL BufferFull(const SPACE * s)
{
  return s->Length == 0;
}

BOOL BufferEmpty(const DATA * d)
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

#ifdef PC_BUILD
BOOL BufferCompare(const DATA * d1, const DATA * d2)
{
  if (d1->Length != d2->Length) {
    return FALSE;
  } else {
    if (memcmp(d1->Buff, d2->Buff, d1->Length) == 0) {
      return TRUE;
    } else {
      return FALSE;
    }
  }
}

void * BufferNextFn(DATA * data, COUNT len) {
  if (len > data->Length) {
    return NULL;
  } else {
    void * place = data->Buff;
    data->Buff += len;
    data->Length -= len;
    return place;
  }
}

#endif //PC_BUILD

void BufferAdvance(const DATA * data, SPACE * space) {
  ASSERT(data->Buff == space->Buff);
  ASSERT(data->Length <= space->Length);
  space->Buff += data->Length;
  space->Length -= data->Length;
}
