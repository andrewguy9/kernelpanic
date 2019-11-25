#ifndef B_HAM_H
#define B_HAM_H

#include"utils.h"

struct BHAM
{
	//Define Slope of Line. The slope (y/x) needs to be less than 1.
	//In other words, x >= y
	COUNT X;
	COUNT Y;
	//Current accumulator
	int Error;
};

void BHamInit( COUNT x, COUNT y, struct BHAM * bham );
_Bool BHamTick( struct BHAM * bham );

#endif
