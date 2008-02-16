#ifndef POSITION_LOG_H
#define POSITION_LOG_H

#include"../utils/flags.h"
#include"../utils/utils.h"

#define MAP_LOG_SIZE(width,height) (FlagSize((width)*(height)))

struct MAP_LOG
{
	COUNT Width;
	COUNT Height;
	FLAG_WORD * Flags;	
};

void MapLogInit( INDEX width, INDEX height, FLAG_WORD * buff, struct MAP_LOG * log );
void MapLogSet( INDEX x, INDEX y, BOOL flagState, struct MAP_LOG * mapLog );
BOOL MapLogGet( INDEX x, INDEX y, struct MAP_LOG * mapLog );

#endif
