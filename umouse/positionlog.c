#include"positionlog.h"

#define XY_TO_INDEX(x, y, posLog) (((y)*posLog->Width)+x)

void MapLogInit( INDEX width, INDEX height, FLAG_WORD * buff, struct MAP_LOG * log )
{
	log->Height = height;
	log->Width = width;
	log->Flags = buff;

	FlagsClear( buff, MAP_LOG_SIZE(width, height) );
}

void MapLogSet( INDEX x, INDEX y, BOOL flagState, struct MAP_LOG * mapLog )
{
	if( flagState )
	{
		FlagOn(mapLog->Flags, XY_TO_INDEX(x,y,mapLog));
	}
	else
	{
		FlagOff(mapLog->Flags, XY_TO_INDEX(x,y,mapLog));
	}
}

BOOL MapLogGet( INDEX x, INDEX y, struct MAP_LOG * mapLog )
{
	return FlagGet( mapLog->Flags, XY_TO_INDEX(x,y,mapLog) );
}

