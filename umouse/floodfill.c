#include"floodfill.h"

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
		*y =FloodFillGetY(index,map);
		*x =FloodFillGetX(index,map);
	}
}

//
//Public Routines
//

unsigned char FloodFillGet( INDEX x, INDEX y, struct FLOOD_MAP * map)
{
	if( x >= map->Width || y >= map->Height )
		return -1;

	return map->FloodMap[FloodFillGetIndex(x,y,map)];
}

void FloodFillInit(
		COUNT width, 
		COUNT height,
	   	char * mapBuff, 
		char * eventBuff,
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
	unsigned char iter;
	for( iter = 0; iter < floodMap->Width*floodMap->Height; iter++ )
	{
		FloodGetEvent(&x, &y, floodMap);

		if( x == -1 && y == -1 )
		{//no events to process
			return;
		}

		if( FloodFillGet( x+1, y+0, floodMap ) > iter )
		{
			FloodFillSet( x+1, y+0, iter, floodMap );
			FloodNewEvent( x+1, y+0, floodMap );
		}
		if( FloodFillGet( x-1, y+0, floodMap ) > iter )
		{
			FloodFillSet( x-1, y+0, iter, floodMap );
			FloodNewEvent( x-1, y+0, floodMap );
		}
		if( FloodFillGet( x+0, y+1, floodMap ) > iter )
		{
			FloodFillSet( x+0, y+1, iter, floodMap );
			FloodNewEvent( x+0, y+1, floodMap );
		}
		if( FloodFillGet( x+0, y-1, floodMap ) > iter )
		{
			FloodFillSet( x+0, y-1, iter, floodMap );
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

