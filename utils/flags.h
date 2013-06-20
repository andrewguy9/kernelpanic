#ifndef FLAGS_H
#define FLAGS_H

/*
 * Presents an abstraction for flags
 */

typedef int FLAG;

#define FlagOn(FIELD, FLAG)     ((FIELD) |= (FLAG))
#define FlagOff(FIELD, FLAG)    ((FIELD) &= (~(FLAG)))
#define FlagToggle(FIELD, FLAG) ((FIELD) ^= (FLAG))
#define FlagGet(FIELD, FLAG)    ((FIELD) &  (FLAG))

#define FlagGetBit(INDEX)       (1<<(INDEX))
#define FlagsEqual(FIELD_A, FIELD_B) ((FIELD_A)==(FIELD_B))

#define FLAG_MAX_INDEX   ((sizeof(FLAG)*8)-1)
#define FLAG_MAX         ((1)<<(FLAG_MAX_INDEX))
#define FLAG_NONE       ((FLAG) (0))
#define FLAG_ALL        ((FLAG)(-1))

#endif //FLAGS_H
