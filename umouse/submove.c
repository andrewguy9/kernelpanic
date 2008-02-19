#include"submove.h"

#include<stdio.h>

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

void SubMoveApply(INDEX * x, INDEX * y, enum DIRECTION * dir, enum SUB_MOVE move, BOOL *translated, BOOL *rotated )
{
	switch(move)
	{
		case SUB_MOVE_FORWARD_WHOLE:
			SubMoveTranslate(x,y,*dir,2);
			*translated = TRUE;
			//printf("forward whole = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_FORWARD_HALF:
			*translated = TRUE;
			SubMoveTranslate(x,y,*dir,1);
			//printf("forward half = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_TURN_RIGHT:
			*rotated = TRUE;
			SubMoveRotate(dir, RIGHT);
			//printf("turn right = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_TURN_LEFT:
			*rotated = TRUE;
			SubMoveRotate(dir, LEFT);
			//printf("turn left = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_TURN_AROUND:
			*rotated = TRUE;
			SubMoveRotate(dir, BACK );
			//printf("turn around = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_INTEGRATE_RIGHT:
			*translated = TRUE;
			*rotated = TRUE;
			SubMoveTranslate(x,y,*dir,1);
			SubMoveRotate(dir, RIGHT);
			SubMoveTranslate( x, y, *dir, 1 );
			//printf("integrate right = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_INTEGRATE_LEFT:
			*translated = TRUE;
			*rotated = TRUE;
			SubMoveTranslate(x,y,*dir,1);
			SubMoveRotate(dir, LEFT);
			SubMoveTranslate( x, y, *dir, 1 );
			//printf("integrate left = %d,%d,%d\n",*x,*y,*dir);
			break;
	}
}
