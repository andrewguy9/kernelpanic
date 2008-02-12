#include"floodfill.h"

//
//Private Helpers
//

//Sets the value of the flood map, and registers "look at me" flag.
void FloodFillSet(INDEX x, INDEX y, unsigned char value, struct FLOOD_MAP * map)
{
	if( x < map->Width && y < map->Height )
		map->FloodMap[y*map->Width+x] = value;
}

//Gets the value from flood map
UNSIGNED CHAR FloodFillGet( INDEX x, INDEX y, struct FLOOD_MAP * map)
{
	if( x < map->Width && y < map->Height )
		return map->FloodMap[y*map->Width + x];
	else 
		return -1
}

void FloodNewEvent( INDEX x, INDEX y, struct FLOOD_MAP * map )
{
	INDEX index = x+y*map->Width;
	if( index < map->Width * map->Height)
		FlagOn( map->EventMap, index);
}

void FloodGetEvent( INDEX * x, INDEX * y, struct FLOOD_MAP * map )
{
	INDEX index;
	index = FlagsGetFirstFlag( 
			map->EventMap, 
			FLOOD_EVENT_SIZE( map->Width, map->Height ) );	
	if( index == -1 )
	{
		*x = -1;
		*y = -1;
	}
	else
	{
		*y = index / map->Width;
		*x = index % map->Height;
	}
}

//
//Public Routines
//

void FloodFillInit(
		COUNT width, 
		COUNT height,
	   	char * mapBuff, 
		char * eventBuff,
		struct FLOOD_MAP * floodMap)
{
	floodMap->Width = width;
	floopMap->Weight = height;
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
		FloodGetEvent(&x, &y, map);

		if( x == -1 && y == -1 )
			return;

		if( FloodFillGet( x+1, y+0, map ) > iter )
		{
			FloodFillSet( x+1, y+0, iter, map );
			FloodNewEvent( x+1, y+0, map );
		}
		if( FloodFillGet( x-1, y+0, map ) > iter )
		{
			FloodFillSet( x-1, y+0, iter, map );
			FloodNewEvent( x-1, y+0, map );
		}
		if( FloodFillGet( x+0, y+1, map ) > iter )
		{
			FloodFillSet( x+0, y+1, iter, map );
			FloodNewEvent( x+0, y+1, map );
		}
		if( FloodFillGet( x+0, y-1, map ) > iter )
		{
			FloodFillSet( x+0, y-1, iter, map );
			FloodNewEvent( x+0, y-1, map );
		}
	}
	return;
}

void FloodFillClear(struct FLOOD_MAP * floodMap)
{
	INDEX x,y;
	for( x=0; x<floodMap->Width, x++ )
		for( y=0; y<floodMap->Height; y++)
		{
			FloodFillSet(x,y,-1,floodMap);
		}

	FlagsClear( floodMap->EventMap, floodMap->Width*floodMap->Height );
}

void FloodFillSetDestination( INDEX x, INDEX y, struct FLOOD_MAP map )
{
	FloodFillSet(x,y,0,map);
	FloodNewEvent(x,y,map);
}

