#include"bham.h"

/*
 * Presents a linear interpolation concept.
 *
 * BHam is an algorithm for drawing lines smoothly.
 * Since pixels are integer and lines need to be smooth, 
 * it calculates the slope of the line in terms of integers,
 * dx and dy. The nice thing about BHam is that it does all
 * of this without using a single divide?
 *
 * This is useful for things other than graphics! 
 * For instance, when using stepper motors you want to keep
 * the motor in a rhythm. 
 *
 * Unlike the graphics scenario, we are only interrested in 
 * lines with a slope less than 1. In other words we want 
 * a signal some percentage of times quieried. 
 * Obviously you cant signal more than 100% of the time.
 *
 * Call BHamInit()
 * To setup a signal.
 * x is the dx of the line.
 * y is the dy of the line.
 * x must be greater than or equal to y.
 *
 * Call BHamTick to query.
 * Returns true to "signal"
 * Returns false to say "no signal"
 */

void BHamInit( COUNT x, COUNT y, struct BHAM * bham )
{
	//Check if BHam slope too steep
	ASSERT( x >= y );

	bham->X = x;
	bham->Y = y;
	bham->Error = -bham->X;
}

BOOL BHamTick( struct BHAM * bham )
{
	bham->Error += bham->Y;
	if( bham->Error > 0 )
	{
		//We step on y. Reset error.
		bham->Error -= bham->X;
		return TRUE;
	}
	else
	{
		//We didn't step on y, return.
		return FALSE;
	}
}

