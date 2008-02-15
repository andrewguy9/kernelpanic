#ifndef HANDLER_H
#define HANDLER_H

#include"../utils/link.h"

typedef void (HANDLER_FUNCTION)( void * Argument );

struct HANDLER_OBJECT
{
	union LINK Link;
	BOOL Enabled;
	HANDLER_FUNCTION * Handler;
	void * Argument;
};

#define HandlerIsRunning( obj ) ( (obj)->Enabled )
#endif
