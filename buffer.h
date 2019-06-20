#include "blocks.h"
#ifndef BUFFER_H
#define BUFFER_H

// TODO use utils version.
#define min(x, y) ((x) > (y) ? (y) : (x))

#define BUFFER(T, CLASS)                                                       \
  struct T##_##CLASS {                                                         \
    T *start;                                                                  \
    T *end;                                                                    \
  }

#define SPACE(T) BUFFER(T, _SPACE)

#define DATA(T) BUFFER(T, _DATA)

#define FROM_ARRAY(array)                                                      \
  { .start = array, .end = array + sizeof(array) / sizeof(*array) }

#define FULL(ST)                                                               \
  lambda(_Bool, (ST space), { return space.start == space.end; })

#define EMPTY(DT)                                                              \
  lambda(_Bool, (DT data), { return data.start == data.end; })

#define WRITE(ST, DT)                                                          \
  lambda(DT, (ST *space, typeof(*space->start) x), {                           \
    if (!FULL(ST)(*space)) {                                                   \
      DT data = {space->start++, space->start + 1};                            \
      *data.start = x;                                                         \
      return data;                                                             \
    } else {                                                                   \
      return (DT){space->start, space->end};                                   \
    }                                                                          \
  })

#define COPY(DT, ST)                                                           \
  lambda(DT, (DT src, ST *dst), {                                              \
    DT data;                                                                   \
    data.start = dst->start;                                                   \
    for (; src.start < src.end && dst->start < dst->end;                       \
         src.start++, dst->start++) {                                          \
      *dst->start = *src.start;                                                \
    }                                                                          \
    data.end = dst->start;                                                     \
    return data;                                                               \
  })

#define BufferForEach(T, DT)                                                   \
  lambda(void, (lambdaRef(fn, void, T), DT data), {                            \
    forPtrs(T)(fn, &data.start, data.end);                                     \
  })

// Consume space in dst with mapped values from src returned as data
// TODO lets fail on uneven buffer sizes for now.
// TODO what if we are mapping a large buffer into a small buffer, can we do it
// in multiple rounds?
#define BufferMap(A, B, SRC_T, MEM_T, OUT_T)                                   \
  lambda(OUT_T, (lambdaRef(fn, B, A), SRC_T src, MEM_T *dst), {                \
    B *mem = dst->start;                                                       \
    size_t length = min(dst->end - dst->start, src.end - src.start);           \
    dst->start += length;                                                      \
    map(A, B)(fn, src.start, mem, length);                                     \
    return (OUT_T){mem, mem + length};                                         \
  })

#define BufferFilter(T, DT, ST)                                                \
  lambda(DT, (lambdaRef(fn, _Bool, T), DT *src, ST *dst), {                    \
    T *start = dst->start;                                                     \
    filterPartial(T)(fn, &src->start, src->end, &dst->start, dst->end);        \
    return (DT) {start, dst->start};                                           \
  })

#define BufferReduce(A, B, DA)                                                 \
  lambda(B, (lambdaRef(fn, B, B, A), DA in, B start), {                        \
    return reduce(A, B)(fn, in.start, in.end - in.start, start);               \
  })

#endif // BUFFER_H
