#include"map.h"
#include"../utils/bitmap.h"
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

/*
 * Bit format of MAP (4X4 Example)
 * 
 *    +--+--+--+--+
 * 03 |  |  |  |  |
 *    +--+--+--+--+
 * 02 |  |  |  |  |
 *    +--+--+--+--+
 * 01 |  |  |  |  |
 *    +--+--+--+--+
 * 00 |  |  |  |  |
 *    +--+--+--+--+
 *     00 01 02 03
 *
 * -- Horizontal Wall
 *  | Vertical Wall
 *  + Peg (Between Walls)
 *
 * The map is broken up into two lists of walls. The V or vertical walls and
 * the H or Horizontal Walls. Walls along the outside of the map are implied.
 *  
 *    +--+--+--+--+
 * 03 |  |  |  |  |
 *    +--+--+--+--+
 * 02 |  |  |  |  |
 *    +--+--+--+--+
 * 01 |  |  |  |  |
 *    +--+--+--+--+
 * 00 |  |  |  |  |
 *    +--+--+--+--+
 *     00 01 02 03
 *
 * Because the walls on the edge are implied we can pull them out of the list.
 * This means that the list for each wall is smaller than the number of walls
 * in the map.
 *
 * Size of H Walls = Width * (Height - 1)
 * Size of V Walls = Height * (Width - 1)
 *
 * Because the direction the walls face determines weather Width or Height is shorter
 * we define a Major and Minor Axis for each list.
 *
 * H Walls - Width = Major, Height = Minor
 * V Walls - Height = Major, Width = Minor
 *
 * H Wall Index = X * (Width-1) + Y
 * V Wall Index = X             + Y * (Height-1)
 *
 * Looking up a wall is a function of the cell and a direction.
 * We convert this into an X Y Lookup on a particular wall list.
 * North and South are Horizontal Walls.
 * East and West are Vertical Walls.
 * For Horizontal Walls North is Y+1.
 * For Vertical Walls East is X+1.
 *
 * The list of walls is laid out in a bitmap. The order for H-Walls is below:
 * Y - Axis
 *    +**+**+**+**+
 * 03 *  |  |  |  *
 *    +02+05+08+0B+
 * 02 *  |  |  |  *
 *    +01+04+07+0A+
 * 01 *  |  |  |  *
 *    +00+03+06+09+
 * 00 *  |  |  |  *
 *    +**+**+**+**+
 *     00 01 02 03 X - Axis
 *
 * The list of walls is laid out in a bitmap. The order for V-Walls is below:
 * Y - Axis
 *    +**+**+**+**+
 * 03 *  9  A  B  *
 *    +--+--+--+--+
 * 02 *  6  7  8  *
 *    +--+--+--+--+
 * 01 *  3  4  5  *
 *    +--+--+--+--+
 * 00 *  0  1  2  *
 *    +**+**+**+**+
 *     00 01 02 03 X - Axis
 */

//
//Private Mathmeatical macros
//

#define MapGetIndex( major, majorSize, minor ) ( (minor)*((majorSize)-1)+(major-1) )
#define MapAdjustX( x, dir ) ( (dir) == (EAST) ? (x)+1 : (x) )
#define MapAdjustY( y, dir ) ( (dir) == (NORTH) ? (y)+1 : (y) )
#define MapOutOfBounds( major, majorSize ) \
	( (major) == 0 || (major) == (majorSize) ? TRUE : FALSE )//TODO BROKEN

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

BOOL MapInit( struct MAP * map, FLAG_WORD * wallBuff, COUNT buffLen, COUNT width, COUNT height )
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

	//Map Get Wall has invalid direction.
	ASSERT(0);
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
