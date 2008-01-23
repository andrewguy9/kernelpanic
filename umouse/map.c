#include"map.h"
#include"../utils/flags.h"
#include"../utils/utils.h"

//
//Definition of dimmensions
//

#define MAP_HEIGHT 16

//
//Private Mathmeatical macros
//

#define MapGetIndex( major, majorSize, minor ) ( (minor)*((majorSize)-1)+(major-1) )
#define MapAdjustX( x, dir ) ( (dir) == (EAST) ? (x)+1 : (x) )
#define MapAdjustY( y, dir ) ( (dir) == (NORTH) ? (y)+1 : (y) )
#define MapOutOfBounds( major, majorSize ) ( (major) == 0 || (major) == (majorSize) ? TRUE : FALSE )

//
//Private functions
//

BOOL MapGetFlag( INDEX major, INDEX majorSize, INDEX minor, FLAG_WORD * walls )
{
	if( MapOutOfBounds(major, majorSize ) )
	{//checking in gauranteed space
		return TRUE;
	}
	else
	{//checking in mapped space
		return FlagGet( walls, MapGetIndex( major, majorSize, minor ) );
	}
}

void MapSetFlag( INDEX major, INDEX majorSize, INDEX minor, FLAG_WORD * walls, BOOL state )
{
	INDEX index;
	if( ! MapOutOfBounds( major, majorSize ) )
	{//Can only set in bounds
		index = MapGetIndex( major, majorSize, minor );
		if( state )
			FlagOn( walls, index );
		else
			FlagOff( walls, index );
	}
}

//
//Public Functions
//

BOOL MapInit( struct MAP * map, char * wallBuff, COUNT buffLen, COUNT width, COUNT height )
{
	INDEX cur;
	if( buffLen >= MapSizeNeeded( width, height ) )
	{
		map->VWalls = wallBuff;
		map->HWalls = &wallBuff[MapDimensionSizeNeeded(width, height)];
		map->Height = height;
		map->Width = width;

		for( cur=0; cur < buffLen; cur++ )
			wallBuff[cur] = 0;

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL MapGetWall( INDEX x, INDEX y, enum DIRECTION dir, struct MAP * map )
{

	x = MapAdjustX(x,dir);
	y = MapAdjustY(y,dir);

	switch( dir )
	{
		case NORTH:
		case SOUTH:
			return MapGetFlag( y, map->Height, x, map->HWalls );
			break;
		case EAST:
		case WEST:
			return MapGetFlag( x, map->Width, y, map->VWalls );
			break;
	}
	ASSERT(0, 0, "Map Get Wall has invalid direction.");
	return FALSE;
}

void MapSetWall( INDEX x, INDEX y, enum DIRECTION dir, BOOL state, struct MAP * map )
{
	x = MapAdjustX(x,dir);
	y = MapAdjustY(y,dir);

	switch(dir)
	{
		case NORTH:
		case SOUTH:
		MapSetFlag( y, map->Height, x, map->HWalls, state );
		break;
		case EAST:
		case WEST:
		MapSetFlag( x, map->Width, y, map->VWalls, state );
		break;
	}
}
