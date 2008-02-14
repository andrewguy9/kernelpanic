#ifndef FLOOD_FILL_H
#define FLOOD_FILL_H

#include"../utils/flags.h"
#include"map.h"
#include"../utils/ringbuffer.h"

#define FLOOD_MAP_SIZE( WIDTH, HEIGHT ) ((WIDTH)*(HEIGHT)*sizeof(char))

#define FLOOD_EVENT_SIZE( WIDTH, HEIGHT ) (FlagSize((WIDTH)*(HEIGHT)))

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
