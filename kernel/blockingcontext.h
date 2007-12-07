#ifndef BLOCKING_CONTEXT_H
#define BLOCKING_CONTEXT_H

#include"resource.h"
#include"handler.h"

union BLOCKING_CONTEXT
{
	struct HANDLER_OBJECT SleepTimer;
	enum RESOURCE_STATE ResourceWaitState;
};

#endif
