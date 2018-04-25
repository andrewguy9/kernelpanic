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
// TODO IS THERE A GOOD WAY TO DO THIS LIKE calloc?
#define BufferSpace(buff, size) {(void*) (buff), (size)}

#ifdef PC_BUILD
//XXX COULD WE MANAGE STRUCT CHARS DISTINCTION IN HERE?
//XXX COULD WE HAVE A VARIENT TO FILL THE BUFFER WITH A PATTERN?
//XXX COULD WE PARAMETRIZE LIKE calloc? I WOULD LIKE TO AMORTIZE LOCK ACQUIRES IN READ/WRITE-STRUCT.
void BufferCopy(DATA * d, SPACE * s);
#endif //PC_BUILD

#define BufferFromObj(obj) {(char*) &(obj), sizeof(obj)}
#define BufferToObj(data, type) (*(type *) data.Buff)
extern const struct BUFFER BufferNull;
//TODO PROTOTYPE. Buff advancement is not safe.
void * BufferNextFn(DATA * data, COUNT len);
#define BufferNext(data, val) ((typeof(val)) BufferNextFn(&data, sizeof(*val)))

//TODO BufferData is inconvient because we need to stash buff.
DATA BufferData(char * buff, const SPACE * s);

#define BufferFull(space) ((space)->Length == 0)
#define BufferEmpty(data) ((data)->Length == 0)

#ifdef PC_BUILD
BOOL BufferPrint(SPACE * s, char * format, ...);
#endif //PC_BUILD

#ifdef PC_BUILD
BOOL BufferCompare(const DATA * d1, const DATA * d2);
#endif //PC_BUILD

#define BufferAdvance(data, space) ({ \
    ASSERT((data)->Buff == (space)->Buff); \
    ASSERT((data)->Length <= (space)->Length); \
    (space)->Buff += (data)->Length; \
    (space)->Length -= (data)->Length; \
    })

#endif //BUFF_H
