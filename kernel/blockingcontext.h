#ifndef BLOCKING_CONTEXT_H
#define BLOCKING_CONTEXT_H

#include"resource.h"

union BLOCKING_CONTEXT
{
	enum RESOURCE_STATE ResourceWaitState;
};

#endif
