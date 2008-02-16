#ifndef POSITION_LOG_H
#define POSITION_LOG_H

#include"../utils/flags.h"
#include"../utils/utils.h"

#define SCAN_LOG_SIZE(width,height) (FlagSize((width)*(height)))

struct SCAN_LOG
{
	COUNT Width;
	COUNT Height;
	FLAG_WORD * Flags;	
};

void ScanLogInit( INDEX width, INDEX height, FLAG_WORD * buff, struct SCAN_LOG * log );
void ScanLogSet( INDEX x, INDEX y, BOOL flagState, struct SCAN_LOG * mapLog );
BOOL ScanLogGet( INDEX x, INDEX y, struct SCAN_LOG * mapLog );

#endif
