#ifndef STACK_H
#define STACK_H

struct STACK
{
	char * Pointer;

#ifdef DEBUG
	char * High;
	char * Low;
#endif
};

#endif
