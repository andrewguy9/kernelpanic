#ifndef MAP_H
#define MAP_H

#include"../utils/utils.h"

//
//Default map height and width
//
#define MAP_WIDTH 16
#define MAP_HEIGHT 16

//
//Public math macros
//

#define MapDimensionSizeNeeded( major, minor ) ( FlagSize( (major) * ((minor)-1)) )

#define MapSizeNeeded( width, height ) \
	( MapDimensionSizeNeeded( width, height) + \
	  MapDimensionSizeNeeded( height, width )
//
//Map structure
//

typedef enum DIRECTION { NORTH, SOUTH, EAST, WEST };

struct MAP
{
	FLAG_WORD VWalls;
	FLAG_WORD HWalls;
	COUNT Height;
	COUNT Width;
}
void MapInit( MAP * map, FLAG_WORD * wallBuff, COUNT buffLen, COUNT height, COUNT width );
BOOL MapGetWall( INDEX x, INDEX y, DIRECTION dir, MAP * map );
void MapSetWall( INDEX x, INDEX y, DIRECTION dir, MAP * map );

#endif
