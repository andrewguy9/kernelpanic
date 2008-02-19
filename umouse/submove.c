#include"submove.h"

#include<stdio.h>

#define DID_ROTATE 0x01
#define DID_TRANSLATE 0x02
//routines to compute position delta
char SubMoveTranslate( INDEX * x, INDEX * y, enum DIRECTION dir, COUNT dist )
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
	return DID_TRANSLATE;
}
	
char SubMoveRotate( enum DIRECTION * dir, enum ANGLE angle )
{
	* dir = TURN((*dir),angle);
	return DID_ROTATE;
}

char SubMoveApply(INDEX * x, INDEX * y, enum DIRECTION * dir, enum SUB_MOVE move)
{
	char flags = 0;
	switch(move)
	{
		case SUB_MOVE_DONE:
			break;
		case SUB_MOVE_START:
			flags |= SubMoveTranslate(x,y,*dir,1);
			//printf("forward start = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_STOP:
			flags |= SubMoveTranslate( x, y, *dir, 1 );
			//printf("forward stop = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_FORWARD:
			flags|=SubMoveTranslate(x,y,*dir,1);
			break;
		case SUB_MOVE_TURN_RIGHT:
			flags|=SubMoveRotate(dir, RIGHT);
			//printf("turn right = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_TURN_LEFT:
			flags|=SubMoveRotate(dir, LEFT);
			//printf("turn left = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_TURN_AROUND:
			flags|=SubMoveRotate(dir, BACK );
			//printf("turn around = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_INTEGRATE_RIGHT:
			flags|=SubMoveTranslate(x,y,*dir,1);
			flags|=SubMoveRotate(dir, RIGHT);
			flags|=SubMoveTranslate( x, y, *dir, 1 );
			//printf("integrate right = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_INTEGRATE_LEFT:
			flags|=SubMoveTranslate(x,y,*dir,1);
			flags|=SubMoveRotate(dir, LEFT);
			flags|=SubMoveTranslate( x, y, *dir, 1 );
			//printf("integrate left = %d,%d,%d\n",*x,*y,*dir);
			break;
	}
	return flags;
}

//Moving state required when starting a move
BOOL SubMoveMovingStartState[9] = 
{FALSE,FALSE,TRUE,TRUE,FALSE,FALSE,FALSE,TRUE,TRUE};
//ending state of a move
BOOL SubMoveMovingEndState[9] = 
{FALSE,TRUE,FALSE,TRUE,FALSE,FALSE,FALSE,TRUE,TRUE};
//wheather a move turns
BOOL SubMovesRotates[9] = 
{FALSE,FALSE,FALSE,FALSE,TRUE,TRUE,TRUE,TRUE,TRUE};


BOOL SubMoveLegal( 
		INDEX x, 
		INDEX y, 
		enum DIRECTION dir, 
		enum SUB_MOVE move,
	   	BOOL moving, 
		struct MAP *map,
	   	struct SCAN_LOG *scan )
{
	char moveFlags;

	//verify that moving start states line up
	if( moving != SubMoveMovingStartState[move] )
		return FALSE;

	moveFlags = SubMoveApply( &x, &y , &dir, move );

	if( moveFlags & DID_TRANSLATE )
	{
		//verify we dont cross wall
		if( MapGetWall( x/2, y/2, TURN(dir,BACK), map ) )
			return FALSE;
		//verify we dont enter unexplored cell.
		if( ! ScanLogGet( x/2, y/2, scan ) )
			return FALSE;
	}

	return TRUE;
}

enum SUB_MOVE SubMoveFindBest(
		INDEX startX,
	   	INDEX startY,
	   	enum DIRECTION startDir,
		BOOL moving,
	   	struct FLOOD_MAP * flood, 
		struct MAP * map,
		struct SCAN_LOG * scan)
{
	//best move vars
	enum SUB_MOVE best;
	unsigned char bestFlood;
	unsigned char bestFacingFlood;
	BOOL bestFacingWall;
	BOOL bestRotated;

	//cur move vars
	enum SUB_MOVE cur;
	unsigned char curFlood;
	unsigned char curFacingFlood;
	BOOL curFacingWall;
	BOOL curRotated;

	//mouse state vars
	INDEX x;
	INDEX y;
	enum DIRECTION dir;

	//set up standard to measure against: SUB_MOVE_DONE
	x = startX;
	y = startY;
	dir = startDir;
	best = SUB_MOVE_DONE;
	bestFlood = FloodFillGet( x/2, y/2, flood );
	bestFacingWall = MapGetWall( x/2, y/2, dir, map );
	SubMoveApply( &x, &y, &dir, SUB_MOVE_FORWARD );
	bestFacingFlood = FloodFillGet( x/2, y/2, flood );
	bestRotated = SubMovesRotates[SUB_MOVE_DONE];

	for( cur = SUB_MOVE_START; cur <= SUB_MOVE_INTEGRATE_LEFT; cur++ )
	{
		//gather state vars
		x = startX;
		y = startY;
		dir = startDir;
		//check move is legal.
		if( ! SubMoveLegal(x, y, dir, cur, moving, map, scan))
			continue;
		//gather results
		curFlood = FloodFillGet( x/2, y/2, flood );
		curFacingWall = MapGetWall( x/2, y/2, dir, map );
		SubMoveApply( &x, &y, &dir, SUB_MOVE_FORWARD );
		curFacingFlood = FloodFillGet( x/2, y/2, flood );
		curRotated = SubMovesRotates[cur];

		//
		//look for improvement.
		//

		//favor least floodfill
		if( curFlood > bestFlood )
			continue;
		//favor facing least floodfill
		if( curFacingFlood > bestFacingFlood)
			continue;
		//favor not facing wall.
		if( curFacingWall && ! bestFacingWall)
			continue;
		//favor facing same direction
		if( !curRotated && bestRotated )
			continue;

		//we are either better or a direct tie! New best
		best = cur;
		bestFlood = curFlood;
		bestFacingWall = curFacingWall;
		bestFacingFlood = curFacingFlood;
		bestRotated = curRotated;
	}
	return best;
}
