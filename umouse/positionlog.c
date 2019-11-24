#include"positionlog.h"

#define XY_TO_INDEX(x, y, posLog) (((y)*posLog->Width)+x)

void ScanLogInit( INDEX width, INDEX height, BITMAP_WORD * buff, struct SCAN_LOG * log )
{
	log->Height = height;
	log->Width = width;
	log->Flags = buff;

	BitmapClear( buff, SCAN_LOG_SIZE(width, height) );
}

void ScanLogSet( INDEX x, INDEX y, _Bool flagState, struct SCAN_LOG * mapLog )
{
	if( flagState )
	{
		BitmapOn(mapLog->Flags, XY_TO_INDEX(x,y,mapLog));
	}
	else
	{
		BitmapOff(mapLog->Flags, XY_TO_INDEX(x,y,mapLog));
	}
}

_Bool ScanLogGet( INDEX x, INDEX y, struct SCAN_LOG * mapLog )
{
	if( x >= mapLog->Width || y >= mapLog->Height )
		return false;//we cannot scan cells outside map.
	return BitmapGet( mapLog->Flags, XY_TO_INDEX(x,y,mapLog) );
}

