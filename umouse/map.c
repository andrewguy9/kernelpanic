#include"map.h"
#include"../utils/flags.h"
#include"../utils/panic.h"

//
//Definition of dimmensions
//

#define MAP_HEIGHT 16

//
//Private Mathmeatical macros
//

#define MapGetIndex( major, majorSize, minor ) ( (minor)*((majorSize)-1)+(major-1) )
#define MapAdjustX( x, dir ) ( (dir) == (EAST) ? (X)+1 : (X) )
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
			FlagSet( walls, index );
		else
			FlagClear( walls, index );
	}
}

//
//Public Functions
//

void MapInit( MAP * map, char * wallBuff, COUNT buffLen, COUNT width, COUNT height )
{
	if( buffLen >= MapSizeNeeded( width, height ) )
	{
		map->VWalls = wallBuff;
		map->HWalls = &wallBuff[MapDimensionSizeNeeded(width), MapDimensionSizeNeeded(height)];
		map->Height = height;
		map->Width = width;
	}
	else
	{
		GeneralPanic( 0 );
	}
}

BOOL MapGetWall( INDEX x, INDEX y, DIRECTION dir, MAP * map )
{

	x = MapAdjustX(x,dir);
	y = MapAdjustY(y,dir);

	switch( dir )
	{
		case NORTH:
		case SOUTH:
			return MapGetFlag( y, map->Height, x, map->Width, & map->HWalls );
			break;
		case EAST:
		case WEST:
			return MapGetFlag( x, map->Width, y, map->Height, & map->VWalls );
			break;
	}
}

void MapSetWall( INDEX x, INDEX y, DIRECTION dir, BOOL state, MAP * map )
{
	INDEX index;
	x = MapAdjustX(x,dir);
	y = MapAdjustY(y,dir);

	switch(dir)
	{
		case NORTH:
		case SOUTH:
		MapSetFlag( y, map->Height, x, map->Width, &map->HWalls, state );
		break;
		case EAST:
		case WEST:
		MapSetFlag( x, map->Width, y, map->Height, &map->VWalls, state );
		break;
	}
}
