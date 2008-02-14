#ifndef FLOOD_FILL_H
#define FLOOD_FILL_H

#include"../utils/flags.h"
#include"map.h"
#include"../utils/ringbuffer.h"


#define FLOOD_MAP_ITEMS_NEEDED( WIDTH, HEIGHT ) ((WIDTH)*(HEIGHT))
#define FLOOD_MAP_SIZE( WIDTH, HEIGHT ) (FLOOD_MAP_ITEMS_NEEDED((WIDTH), (HEIGHT)))

#define FLOOD_EVENT_SIZE( WIDTH, HEIGHT ) (FlagSize(FLOOD_MAP_ITEMS_NEEDED((WIDTH),(HEIGHT))))

#define FloodFillGetIndex(x,y,map) (((y)*(map)->Width)+(x))

#define FloodFillGetX(index, map) (  (index) % (map->Height) )
#define FloodFillGetY(index, map) ( (index) / (map->Width))

struct FLOOD_MAP 
{
	COUNT Width;
	COUNT Height;
	char * FloodMap;
	char * EventMap;
};

void FloodFillInit(
		COUNT width, 
		COUNT height,
	   	char * mapBuff, 
		char * eventBuff,
		struct FLOOD_MAP * floodMap);

void FloodFillCalculate(
		struct MAP * map,
		struct FLOOD_MAP * floodMap );

void FloodFillClear(struct FLOOD_MAP * floodMap);

void FloodFillSetDestination( INDEX x, INDEX y, struct FLOOD_MAP * map );

unsigned char FloodFillGet( INDEX x, INDEX y, struct FLOOD_MAP * map);
#endif
