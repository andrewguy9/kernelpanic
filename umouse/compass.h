#ifndef COMPASS_H
#define COMPASS_H

enum DIRECTION { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };
enum ANGLE { STRAIGHT = 0, RIGHT = 1, BACK = 2, LEFT = 3 };

#define DIR_MASK 0x03
#define TURN( dir, angle ) ( ((dir)+(angle)) & DIR_MASK )

#endif
