#ifndef BITFIELD_H
#define BITFIELD_H

/*
 * Presents an abstraction for single word bitmaps (i.e. flags).
 */

typedef int BITFIELD;

#define FlagOn(FIELD, FLAG)     ((FIELD) |= (FLAG))
#define FlagOff(FIELD, FLAG)    ((FIELD) &= (~(FLAG)))
#define FlagToggle(FIELD, FLAG) ((FIELD) ^= (FLAG))
#define FlagGet(FIELD, FLAG)    ((FIELD) &  (FLAG))
#define FlagGetBit(INDEX)       (1<<(INDEX))
#define FlagsEqual(FIELD_A, FIELD_B) ((FIELD_A)==(FIELD_B))

#define FLAG_MAX_INDEX   ((sizeof(BITFIELD)*8)-1)
#define FLAG_MAX         ((1)<<(FLAG_MAX_INDEX))
#define FLAG_NONE       ((BITFIELD) (0))
#define FLAG_ALL        ((BITFIELD)(-1))

#endif //BITFIELD_H
