#include"submove.h"

void SubMoveTranslate( INDEX * x, INDEX * y, enum DIRECTION dir, COUNT dist )
{
	switch(dir)
	{
		case NORTH:
			*y+=dist;
			break;
		case SOUTH:
			*y-=dist;
			break;
		case EAST:
			*x+=dist;
			break;
		case WEST:
			*x-=dist;
			break;
	}
}
	
void SubMoveRotate( enum DIRECTION * dir, enum ANGLE angle )
{
	*dir = TURN(*dir,angle);
}
