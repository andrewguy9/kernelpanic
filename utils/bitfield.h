#ifndef BITFIELD_H
#define BITFIELD_H

typedef int BITFIELD;

#define FlagOn(FIELD, FLAG)     ((FIELD) |= (FLAG))
#define FlagOff(FIELD, FLAG)    ((FIELD) &= (~(FLAG)))
#define FlagToggle(FIELD, FLAG) ((FIELD) ^= (FLAG))
#define FlagGet(FIELD, FLAG)    ((FIELD) &  (FLAG))

#endif //BITFIELD_H
