#include"floodfill.h"

//
//Private Helpers
//

//Sets the value of the flood map, and registers "look at me" flag.
void FloodFillSet(INDEX x, INDEX y, unsigned char value, struct FLOOD_MAP * map)
{
	map->FloodMap[y*map->Width+x] = value;
}

//Gets the value from flood map
UNSIGNED CHAR FloodFillGet( INDEX x, INDEX y, struct FLOOD_MAP * map)
{
	return map->FloodMap[y*map->Width + x];
}

void FloodNewEvent( INDEX x, INDEX y, struct FLOOD_MAP * map )
{
	INDEX index = x+y*map->Width;
	FlagOn( map->EventMap, index);
}

void FloodGetEvent( INDEX * x, INDEX * y, struct FLOOD_MAP * map )
{
	INDEX index;
	FlagsGetFirstFlag( map->EventMap, FLOOD_EVENT_SIZE( map->Width, map->Height ) );	
	*y = index / map->Width;
	*x = index % map->Height;
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
		
}

void FloodFillClear(struct FLOOD_MAP * floodMap)
{
	INDEX x,y;
	for( x=0; x<floodMap->Width, x++ )
		for( y=0; y<floodMap->Height; y++)
		{
			FloodFillSet(x,y,-1,floodMap);
		}
}

void FloodFillSetDestination( INDEX x, INDEX y, struct FLOOD_MAP map )
{
	FloodFillSet(x,y,0,map);
	FloodNewEvent(x,y,map);
}

