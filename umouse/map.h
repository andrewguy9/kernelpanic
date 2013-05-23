#ifndef MAP_H
#define MAP_H

#include"utils/utils.h"
#include"utils/bitmap.h"
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
#define MapDimensionSizeNeeded( major, minor ) ( BitmapSize(MapBitsNeeded(major,minor)) )

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
	BITMAP_WORD * VWalls;
	BITMAP_WORD * HWalls;
	COUNT Height;
	COUNT Width;
};

BOOL MapInit( struct MAP * map, BITMAP_WORD * wallBuff, COUNT buffLen, COUNT height, COUNT width );
BOOL MapGetWall( INDEX x, INDEX y, enum DIRECTION dir, struct MAP * map );
void MapSetWall( INDEX x, INDEX y, enum DIRECTION dir, BOOL state, struct MAP * map );

#endif
