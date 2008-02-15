#ifndef MAP_H
#define MAP_H

#include"../utils/utils.h"
#include"../utils/flags.h"
//
//Default map height and width
//
#define MAP_WIDTH 16
#define MAP_HEIGHT 16

//
//Private math macros
//

#define MapDimensionSizeNeeded( major, minor ) ( FlagSize( (major) * ((minor)-1)) )

//
//Public Math Macros
//

#define MapSizeNeeded( width, height ) \
	( MapDimensionSizeNeeded( width, height) + \
	  MapDimensionSizeNeeded( height, width ) )

//
//Map structure
//

enum DIRECTION { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };
enum ANGLE { STRAIGHT = 0, RIGHT = 1, BACK = 2, LEFT = 3 };
#define DIR_MASK 0x03
#define TURN( dir, angle ) ( ((dir)+(angle)) & DIR_MASK )
struct MAP
{
	FLAG_WORD * VWalls;
	FLAG_WORD * HWalls;
	COUNT Height;
	COUNT Width;
};

BOOL MapInit( struct MAP * map, FLAG_WORD * wallBuff, COUNT buffLen, COUNT height, COUNT width );
BOOL MapGetWall( INDEX x, INDEX y, enum DIRECTION dir, struct MAP * map );
void MapSetWall( INDEX x, INDEX y, enum DIRECTION dir, BOOL state, struct MAP * map );

#endif
