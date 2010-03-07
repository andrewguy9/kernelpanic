#include"floodfill.h"

#ifdef PC_BUILD
#include<stdio.h>
#endif

/*
 * Calculates the number of moves from the "destination" for a given map.
 *
 * Usage:
 * Call FloodFillInit(), 
 * 	with sizeof(mapBuff) = FLOOD_MAP_SIZE() and 
 * 	sizeof(eventMap) = FLOOD_EVENT_SIZE.
 * This will initialize a flood map to solve the maze.
 *
 * After setting up init, you need to set the destination.
 * Call FloodFillSetDestination() for each cell you want to
 * mark as destination.
 *
 * Once the destination is set we can solve the maze.
 * Call FloodGillCalculate()
 * This is a heavy operation, but after the maze will be solved.
 *
 * Call FloodFillGet() to fetch the distance from the destination for a cell.
 */

//
//Private Helpers
//

//Sets the value of the flood map, and registers "look at me" flag.
void FloodFillSet(INDEX x, INDEX y, unsigned char value, struct FLOOD_MAP * map)
{
	if( x >= map->Width || y >= map->Height )
		return;
	map->FloodMap[FloodFillGetIndex(x,y,map)] = value;
}

//Gets the value from flood map
void FloodNewEvent( INDEX x, INDEX y, struct FLOOD_MAP * map )
{
	INDEX index;
	if( x >= map->Width || y >= map->Height )
		return;
	index = FloodFillGetIndex(x,y,map);
	FlagOn( map->EventMap, index );
}

void FloodGetEvent( INDEX * x, INDEX * y, struct FLOOD_MAP * map )
{
	INDEX index;
	index = FlagsGetFirstFlag( 
			map->EventMap, 
			FLOOD_MAP_ITEMS_NEEDED( map->Width, map->Height ) );

	if( index == -1 )
	{
		*x = -1;
		*y = -1;
	}
	else
	{
		*y = FloodFillGetY(index,map);
		*x = FloodFillGetX(index,map);

		//turn off flag for event
		FlagOff(map->EventMap, index);
	}
}

//
//Public Routines
//

unsigned char FloodFillGet( INDEX x, INDEX y, struct FLOOD_MAP * map)
{
	if( x >= map->Width || y >= map->Height )
		return 255;

	return map->FloodMap[FloodFillGetIndex(x,y,map)];
}

void FloodFillInit(
		COUNT width, 
		COUNT height,
	   	FLAG_WORD * mapBuff, 
		FLAG_WORD * eventBuff,
		struct FLOOD_MAP * floodMap)
{
	floodMap->Width = width;
	floodMap->Height = height;
	floodMap->FloodMap = mapBuff;
	floodMap->EventMap = eventBuff;
}

void FloodFillCalculate(
		struct MAP * map,
		struct FLOOD_MAP * floodMap )
{
	INDEX x,y;
	unsigned char curValue;
	while(TRUE)
	{
#ifdef PC_BUILD
		//printf("start\n");
#endif
		//FlagsPrint( floodMap->EventMap, FLOOD_MAP_ITEMS_NEEDED(floodMap->Width, floodMap->Height) );
		FloodGetEvent(&x, &y, floodMap);

		if( x == -1 && y == -1 )
		{//no events to process
#ifdef PC_BUILD
			//printf("no events\n");
#endif
			return;
		}

		curValue = FloodFillGet( x,y, floodMap );
#ifdef PC_BUILD
		//printf("curValue = %d", curValue );
#endif

		if( ! MapGetWall( x, y, EAST, map ) && 
				FloodFillGet( x+1, y+0, floodMap ) > curValue )
		{
			FloodFillSet( x+1, y+0, curValue+1, floodMap );
			FloodNewEvent( x+1, y+0, floodMap );
		}
		if( ! MapGetWall( x, y, WEST, map ) &&
				FloodFillGet( x-1, y+0, floodMap ) > curValue )
		{
			FloodFillSet( x-1, y+0, curValue+1, floodMap );
			FloodNewEvent( x-1, y+0, floodMap );
		}
		if( ! MapGetWall( x, y, NORTH, map ) &&
				FloodFillGet( x+0, y+1, floodMap ) > curValue )
		{
			FloodFillSet( x+0, y+1, curValue+1, floodMap );
			FloodNewEvent( x+0, y+1, floodMap );
		}
		if( ! MapGetWall( x, y, SOUTH, map ) &&
				FloodFillGet( x+0, y-1, floodMap ) > curValue )
		{
			FloodFillSet( x+0, y-1, curValue+1, floodMap );
			FloodNewEvent( x+0, y-1, floodMap );
		}
	}
	return;
}

void FloodFillClear(struct FLOOD_MAP * floodMap)
{
	INDEX x,y;
	//Clear the flood map
	for( x=0; x<floodMap->Width; x++ )
		for( y=0; y<floodMap->Height; y++)
		{
			FloodFillSet(x,y,-1,floodMap);
		}

	//Clear the events flags.
	FlagsClear( floodMap->EventMap, floodMap->Width*floodMap->Height );
}

void FloodFillSetDestination( INDEX x, INDEX y, struct FLOOD_MAP * map )
{
	FloodFillSet(x,y,0,map);
	FloodNewEvent(x,y,map);
}

