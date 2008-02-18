#include"map.h"
#include"../utils/flags.h"
#include"../utils/utils.h"

/*
 * Exposes a map concept for the mouse.
 *
 * Call MapInit() 
 * With sizeof(wallbuff) = MapSizeNeeded()
 * This will initialize a map structure for use later.
 *
 * Call MapGetWall() to fetch wheather a wall is present.
 * Returns TRUE if there is a wall.
 * Returns FALSE if there no wall.
 *
 * Call MapSetWall() to set a wall's state in the map.
 * Pass state = TRUE to turn the wall on.
 * Pass state = FALSE to turn the wall off.
 *
 * Both MapGetWall and MapSetWall can be called for indexes out
 * of bounds.
 */

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
#define MapOutOfBounds( major, majorSize ) ( (major) == 0 || (major) == (majorSize) ? TRUE : FALSE )//TODO BROKEN

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
	if( ! MapOutOfBounds( major, majorSize ) )//TODO MAYBE WE SHOULD GAURD EARLIER
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
	if( x > map->Width || y > map->Height )
	{
		//we are out of bounds, assume there is a wall.
		return TRUE;
	}

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

	if( x >= map->Width || y >= map->Height )//TODO IN CONFLICT WITH OTHER CHECK, PICK ONE
		return;//cannot write out of bounds

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
