#include"positionlog.h"

#define XY_TO_INDEX(x, y, posLog) (((y)*posLog->Width)+x)

void ScanLogInit( INDEX width, INDEX height, FLAG_WORD * buff, struct SCAN_LOG * log )
{
	log->Height = height;
	log->Width = width;
	log->Flags = buff;

	FlagsClear( buff, SCAN_LOG_SIZE(width, height) );
}

void ScanLogSet( INDEX x, INDEX y, BOOL flagState, struct SCAN_LOG * mapLog )
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

BOOL ScanLogGet( INDEX x, INDEX y, struct SCAN_LOG * mapLog )
{
	if( x >= mapLog->Width || y >= mapLog->Height )
		return FALSE;//we cannot scan cells outside map.
	return FlagGet( mapLog->Flags, XY_TO_INDEX(x,y,mapLog) );
}

