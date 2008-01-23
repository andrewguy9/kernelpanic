#include"bham.h"

void BHamInit( COUNT x, COUNT y, struct BHAM * bham )
{
	ASSERT( x >= y, 0, "BHam slope too steep" );

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

