#ifndef MAP_H
#define MAP_H

#include"../utils/utils.h"
#include"../utils/bitmap.h"
#include"compass.h"

//
//Default map height and width
//
#define MAP_WIDTH 4
#define MAP_HEIGHT 5

//
//Private math macros
//

#define MapBitsNeeded(major, minor) ((major)*((minor)-1))
#define MapDimensionSizeNeeded( major, minor ) ( FlagSize(MapBitsNeeded(major,minor)) )

//
//Public Math Macros
//

#define MapSizeNeeded( width, height ) \
	( MapDimensionSizeNeeded( width, height) + \
	  MapDimensionSizeNeeded( height, width ) )

//
//Map structure
//

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
