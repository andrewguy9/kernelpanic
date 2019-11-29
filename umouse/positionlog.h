#ifndef POSITION_LOG_H
#define POSITION_LOG_H

#include"utils/bitmap.h"
#include"utils/utils.h"

#define SCAN_LOG_SIZE(width,height) (BitmapSize((width)*(height)))

struct SCAN_LOG
{
	COUNT Width;
	COUNT Height;
	BITMAP_WORD * Flags;	
};

void ScanLogInit( INDEX width, INDEX height, BITMAP_WORD * buff, struct SCAN_LOG * log );
void ScanLogSet( INDEX x, INDEX y, _Bool flagState, struct SCAN_LOG * mapLog );
_Bool ScanLogGet( INDEX x, INDEX y, struct SCAN_LOG * mapLog );

#endif
