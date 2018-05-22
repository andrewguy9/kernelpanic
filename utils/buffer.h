#ifndef BUFFER_H
#define BUFFER_H

#include"utils.h"

/*
 * Array is a wrapper around C arrays.
 */
// Define custom types to describe Arrays of T into a Bunch.
#define ARRAY_TYPE(T, Count) \
  struct { \
    T Bunch[(Count)]; \
  }

#define ARRAY_INIT(array, ...) \
  array = (typeof(array)) \
  { \
    .Bunch = {__VA_ARGS__} \
  }

#define ARRAY_COUNT(Array) \
  (sizeof((Array).Bunch)/sizeof(*(Array).Bunch))

/*
 * Cursor is a pair of pointers used to walk an Array
 */
// Define custom types to describe Start and End markers in a Bunch.
#define CURSOR_TYPE(T) \
  struct { \
    T * Start; \
    T * End; \
  }

#define CURSOR_SPACE(cursor, array) \
  cursor = (typeof(cursor)) \
  { \
    .Start = (array).Bunch, \
    .End   = (array).Bunch +  ARRAY_COUNT(array) \
  }

//XXX Uses a GNU extension, 'compound statement'. Not ANSI portable.
#define CURSOR_WRITE(object, cursor) \
  ( \
    ((cursor).Start < (cursor).End) ? \
    ({*((cursor).Start++) = (object); TRUE;}) : \
    FALSE \
    )

#define CURSOR_DATA(CURSOR_T, Array, Space) \
  (CURSOR_T) \
  { \
    .Start = (Array).Bunch, \
    .End   = (Space).Start \
  }

#define CURSOR_READ(cursor) \
  (cursor.Start < cursor.End) ? \
  (cursor).Start++ : \
  NULL

#define CURSOR_FOR_EACH(cursor, item) \
  for (typeof((cursor).Start) item = (cursor).Start; \
      (cursor).Start < (cursor).End; \
      (item) = (cursor).Start = (cursor).Start + 1)

#define CURSOR_COPY(src, dst) \
  do { \
    for (; \
        (src).Start < (src).End && (dst).Start < (dst).End; \
        (src).Start++, (dst).Start++) { \
      *(dst).Start = *(src).Start; \
    } \
  } while (0)

#define CURSOR_FULL(cursor) \
  ((cursor).Start == (cursor).End)

#define CURSOR_EMPTY(cursor) \
  CURSOR_FULL(cursor)

#define CURSOR_UNTIL_FULL(cursor) \
  while (! CURSOR_FULL(cursor))

/*
 * Buffer States
 * Declaration -> Initialization -> Writes -> Finalized -> Reads
 *                /\                                       |
 *                 |---------------------------------------|
 */
#define BUFFER_TYPE(ARRAY_T, CURSOR_T) \
  struct { \
    ARRAY_T Memory; \
    CURSOR_T Space; \
  }

#define BUFFER_INIT(buffer) \
  buffer = (typeof(buffer)) \
  { \
    .Space = CURSOR_SPACE( (buffer).Space, (buffer).Memory) \
  }

#define BUFFER_DATA(buffer) \
  CURSOR_DATA(typeof((buffer).Space), (buffer).Memory, (buffer).Space) \

#define BUFFER_UNTIL_FULL(buffer) \
  CURSOR_UNTIL_FULL((buffer).Space)

#define BUFFER_WRITE(object, buffer) \
  CURSOR_WRITE(object, (buffer).Space);

#define BUFFER_FULL(buffer) \
  CURSOR_FULL((buffer).Space)

#define BUFFER_COPY(srcCursor, dstBuff) \
  CURSOR_COPY(srcCursor, (dstBuff).Space)

#define BUFFER_FINALIZE(buffer) \
  do { \
    (buffer).Space.End = (buffer).Space.Start; \
  } while(0)

/*TODO DoubleBuffer consumes on read, unlike buffer.
 * Do we want flip to be called by user, when done?
 */
#define DOUBLE_BUFFER_TYPE(BUFFER_T, CURSOR_T) \
  struct { \
    BUFFER_T \
      Buff1, Buff2, \
      * ReadBuffer, * WriteBuffer; \
    CURSOR_T Data; \
  }

#define DOUBLE_BUFFER_INIT(dbuffer) \
  dbuffer = (typeof(dbuffer)) \
  { \
    .Buff1 = BUFFER_INIT((dbuffer).Buff1), \
    .Buff2 = BUFFER_INIT((dbuffer).Buff2), \
    .ReadBuffer = &(dbuffer).Buff1, \
    .WriteBuffer = &(dbuffer).Buff2, \
    .Data = { .Start = NULL, .End = NULL } \
  }

#define SWAP(a, b) \
  do { \
    typeof(a) tmp = (a); \
    (a) = (b); \
    (b) = tmp; \
  } while (0)

//TODO CAN CASTS MOVE INTO INIT FTNS?
#define DOUBLE_BUFFER_FLIP(BUFFER_T, CURSOR_T, dbuffer) \
  do { \
    BUFFER_FINALIZE(*(dbuffer).WriteBuffer); \
    (dbuffer).Data = BUFFER_DATA(*((dbuffer).WriteBuffer)); \
    *(dbuffer).ReadBuffer = BUFFER_INIT(*(dbuffer).ReadBuffer); \
    SWAP((dbuffer).ReadBuffer, (dbuffer).WriteBuffer); \
  } while (0)

//XXX Uses a GNU extension, 'compound statement'. Not ANSI portable.
#define DOUBLE_BUFFER_READ(BUFFER_T, CURSOR_T, dbuffer) \
  ({ \
   if (CURSOR_EMPTY((dbuffer).Data)) { \
     DOUBLE_BUFFER_FLIP(BUFFER_T, CURSOR_T, dbuffer);\
   } \
   CURSOR_READ((dbuffer).Data); \
  })

#define DOUBLE_BUFFER_WRITE(object, dbuffer) \
  BUFFER_WRITE(object, *(dbuffer).WriteBuffer)

//TODO map :: (a -> b) -> [a] -> [b]
//TODO foldl :: (a -> b -> a) -> a -> [b] -> a
//TODO iterate :: (a -> a) -> a -> [a]
#endif
