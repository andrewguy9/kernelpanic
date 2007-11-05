#ifndef HANLDER_H
#define HANDLER_H

#include"../utils/link.h"

typedef void (HANDLER_FUNCTION)( void * Argument );

struct HANDLER_OBJECT
{
	struct LINK Link;
	BOOL Enabled;
	TIMER_HANDLER * Handler;
	void * Argument;
};

#endif
