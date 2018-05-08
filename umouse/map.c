#include"map.h"
#include"utils/bitmap.h"
#include"utils/utils.h"

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
 * Because the walls on the edge are implied we can pull them out of the list.
 * This means that the list for each wall is smaller than the number of walls
 * in the map.
 *
 * Size of H Walls = Width  * (Height - 1)
 * Size of V Walls = Height * (Width - 1)
 *
 * Because the direction the walls face determines weather Width or Height is shorter
 * we define a Major and Minor Axis for each list.
 *
 * H Walls - Width(X)  = Major, Height(Y) = Minor
 * V Walls - Height(Y) = Major, Width(X)  = Minor
 *
 * Looking up a wall is a function of the cell and a direction.
 * We convert this into an X Y Lookup on a particular wall list.
 * North and South are Horizontal Walls.
 * East and West are Vertical Walls.
 * For Horizontal Walls North is Y+1.
 * For Vertical Walls East is X+1.
 *
 * The list of walls is laid out in a bitmap. The order for H-Walls is below:
 * Y - Axis (Minor)
 *    +**+**+**+**+
 * 03 *  |  |  |  *
 *    +02+05+08+0B+
 * 02 *  |  |  |  *
 *    +01+04+07+0A+
 * 01 *  |  |  |  *
 *    +00+03+06+09+
 * 00 *  |  |  |  *
 *    +**+**+**+**+
 *     00 01 02 03 X - Axis (Major)
 *
 * The list of walls is laid out in a bitmap. The order for V-Walls is below:
 * Y - Axis (Major)
 *    +**+**+**+**+
 * 03 *  9  A  B  *
 *    +--+--+--+--+
 * 02 *  6  7  8  *
 *    +--+--+--+--+
 * 01 *  3  4  5  *
 *    +--+--+--+--+
 * 00 *  0  1  2  *
 *    +**+**+**+**+
 *     00 01 02 03 X - Axis (Minor)
 *
 * Walls inside the border are in bounds, walls outside are out of bounds.
 * Out of bounds walls are implied to be TRUE.
 * H Walls Out of Bounds Minor = (Y==0) || (Y>=Height)
 * V Walls Out of Bounds Minor = (X==0) || (X>=Width )
 * Generic Out of Bounds Minor = (minor==0) || (minor>=minor_size)
 *
 * H Walls Out of Bounds Major = (X>=Width )
 * V Walls Out of Bounds Major = (Y>=Height)
 * Generic Out of Bounds Major = (major>=major_size)
 * 
 * Each possible wall in bounds is assigned to a bit in the bitmap.
 * H Wall Index =  X    * (Width-1) + (Y-1)
 * V Wall Index = (X-1)             +  Y    * (Height-1)
 * Generic Index = (major) * (MinorSize-1) +(minor-1)
 */

//
//Private Mathmeatical macros
//

#define MapAdjustX( x, dir ) ( (dir) == (EAST) ? (x)+1 : (x) )
#define MapAdjustY( y, dir ) ( (dir) == (NORTH) ? (y)+1 : (y) )
#define MapOutOfBoundsMinor( minor, minorSize ) \
	( (minor) == 0 || (minor) >= (minorSize) ? TRUE : FALSE )
#define MapOutOfBoundsMajor( major, majorSize) \
	 ((major)>=(majorSize))
#define MapGetIndex( major, minorSize, minor ) \
	( (major)*((minorSize-1))+(minor)-1 )

//
//Private functions
//

BOOL MapGetFlag( INDEX major, INDEX majorSize, INDEX minor, INDEX minorSize, BITMAP_WORD * walls )
{
	INDEX index;
	BOOL result;
	if( MapOutOfBoundsMinor(minor, minorSize ) || MapOutOfBoundsMajor(major, majorSize ) )
	{//checking in gauranteed space
		result = TRUE;
	}
	else
	{//checking in mapped space
		index = MapGetIndex( major, minorSize, minor);
		result = BitmapGet( walls, index );
	}
	return result;
}

void MapSetFlag( INDEX major, INDEX majorSize, INDEX minor, INDEX minorSize, BITMAP_WORD * walls, BOOL state )
{
	INDEX index;
	if( ! (MapOutOfBoundsMinor( minor, minorSize ) || MapOutOfBoundsMajor(major, majorSize ) ) )
	{//Can only set in bounds
		index = MapGetIndex( major, minorSize, minor );
		if( state )
			BitmapOn( walls, index );
		else
			BitmapOff( walls, index );
	}
}

//
//Public Functions
//

//TODO Could be converted to use Buffers.
BOOL MapInit( struct MAP * map, BITMAP_WORD * wallBuff, COUNT buffLen, COUNT width, COUNT height )
{
	INDEX cur;
	if( buffLen >= MapSizeNeeded( width, height ) )
	{
		map->VWalls = wallBuff;
		map->HWalls = &wallBuff[MapDimensionSizeNeeded(width, height)];
		map->Height = height;
		map->Width = width;

                //TODO for loop over wallBuff, but should be buffer based.
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
			//H Walls - Width(X)  = Major, Height(Y) = Minor
			return MapGetFlag( x, map->Width, y, map->Height, map->HWalls );
			break;
		case EAST:
		case WEST:
			//V Walls - Height(Y) = Major, Width(X)  = Minor
			return MapGetFlag( y, map->Height, x, map->Width, map->VWalls );
			break;
	}

	//Map Get Wall has invalid direction.
	ASSERT(0);
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
			//H Walls - Width(X)  = Major, Height(Y) = Minor
			MapSetFlag( x, map->Width, y, map->Height, map->HWalls, state );
			break;
		case EAST:
		case WEST:
			//V Walls - Height(Y) = Major, Width(X)  = Minor
			MapSetFlag( y, map->Height, x, map->Width, map->VWalls, state );
		break;
	}
}
