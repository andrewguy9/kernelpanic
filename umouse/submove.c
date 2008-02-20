#include"submove.h"

#include<stdio.h>

//Moving state required when starting a move
BOOL SubMoveMovingStartState[9] = 
{FALSE,FALSE,TRUE,TRUE,FALSE,FALSE,FALSE,TRUE,TRUE};
//ending state of a move
BOOL SubMoveMovingEndState[9] = 
{FALSE,TRUE,FALSE,TRUE,FALSE,FALSE,FALSE,TRUE,TRUE};
//wheather a move turns
BOOL SubMovesRotates[9] = 
{FALSE,FALSE,FALSE,FALSE,TRUE,TRUE,TRUE,TRUE,TRUE};
//wheater a move must be started from center of cell.
BOOL SubMovesStartCentered[9] = 
{FALSE,TRUE,FALSE,FALSE,TRUE,TRUE,TRUE,FALSE,FALSE};

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

char SubMoveApply(INDEX * x, INDEX * y, enum DIRECTION * dir, BOOL * moving, enum SUB_MOVE move)
{
	char flags = 0;
	*moving = SubMoveMovingEndState[move];
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

BOOL SubMoveLegal( 
		INDEX x, 
		INDEX y, 
		enum DIRECTION dir, 
		BOOL moving,
		enum SUB_MOVE move,
		struct MAP *map,
	   	struct SCAN_LOG *scan )
{
	char moveFlags;

	printf("testing move %d\n", move);

	//verify that moving start states line up
	if(  (moving && ! SubMoveMovingStartState[move]) || 
		 (! moving && SubMoveMovingStartState[move] ) )
	{
		printf("wrong moving state\n");
		return FALSE;
	}

	//verify we are in correct position to start move
	if( SubMovesStartCentered[move] && (x%2==0 || y%2==0) ) 
	{
		printf("not in center\n");
		return FALSE;
	}

	//perform move
	moveFlags = SubMoveApply( &x, &y, &dir, &moving, move );

	if( moveFlags & DID_TRANSLATE )
	{
		//verify we dont cross wall
		if( MapGetWall( x/2, y/2, TURN(dir,BACK), map ) )
		{
			printf("crossed wall\n");
			return FALSE;
		}
		//verify we dont enter unexplored cell.
		if( ! ScanLogGet( x/2, y/2, scan ) )
		{
			printf("into unscanned area\n");
			return FALSE;
		}
		//if we end the move 
		//and we are still moving and we are centered,
		//we better not be facing a wall.
		if( SubMoveMovingEndState[move] && (x%2==1 && y%2==1 ))
		{
			if( MapGetWall( x/2, y/2, dir, map ))
			{
				printf("chicken with wall\n");
				return FALSE;
			}
			if( ! ScanLogGet( x/2, y/2, scan ) )
			{
				printf("chicken with a unexplored cell\n");
				return FALSE;
			}
		}
	}

	printf("match!\n");
	return TRUE;
}

enum SUB_MOVE SubMoveFindBest(
		INDEX startX,
	   	INDEX startY,
	   	enum DIRECTION startDir,
		BOOL startMoving,
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
	BOOL moving;

	//set up standard to measure against: SUB_MOVE_DONE
	x = startX;
	y = startY;
	dir = startDir;
	moving = startMoving;
	best = SUB_MOVE_DONE;
	bestFlood = 255;//FloodFillGet( x/2, y/2, flood );
	bestFacingWall = TRUE;//MapGetWall( x/2, y/2, dir, map );
	//SubMoveApply( &x, &y, &dir, &moving, SUB_MOVE_FORWARD );
	bestFacingFlood = 255;//FloodFillGet( x/2, y/2, flood );
	bestRotated = TRUE;//SubMovesRotates[SUB_MOVE_DONE];

	for( cur = SUB_MOVE_START; cur <= SUB_MOVE_INTEGRATE_LEFT; cur++ )
	{
		//gather state vars
		x = startX;
		y = startY;
		dir = startDir;
		moving = startMoving;
		//check move is legal.
		if( ! SubMoveLegal(x, y, dir, moving, cur, map, scan))
			continue;
		//perform move
		SubMoveApply( &x, &y, &dir, &moving, cur );
		//gather results
		curFlood = FloodFillGet( x/2, y/2, flood );
		curFacingWall = MapGetWall( x/2, y/2, dir, map );
		//translate for scan lookup
		SubMoveTranslate( &x, &y, dir, SUB_MOVE_FORWARD );
		curFacingFlood = FloodFillGet( x/2, y/2, flood );
		curRotated = SubMovesRotates[cur];

		//
		//look for improvement.
		//

		//favor least floodfill
		if( curFlood > bestFlood )
		{
			printf("lost on flood\n");
			continue;
		}
		else if( curFlood < bestFlood )
			goto comparisonDone;

		//favor facing least floodfill
		if( curFacingFlood > bestFacingFlood)
		{
			printf("lost on facing flood\n");
			continue;
		}
		else if( curFacingFlood < bestFacingFlood )
			goto comparisonDone;

		//favor not facing wall.
		if( curFacingWall && ! bestFacingWall)
		{
			printf("lost on facing wall\n");
			continue;
		}
		else if( !curFacingWall && bestFacingWall)
			goto comparisonDone;

		//favor facing same direction
		if( !curRotated && bestRotated )
		{
			printf("lost on rotation\n");
			continue;
		}

comparisonDone:
		//we are either better or a direct tie! New best
		best = cur;
		bestFlood = curFlood;
		bestFacingWall = curFacingWall;
		bestFacingFlood = curFacingFlood;
		bestRotated = curRotated;
		printf("%d took the lead with ff=%d,facing=%d,fwall=%d,rot=%d\n",
				best,
				bestFlood,
				bestFacingFlood,
				bestFacingWall,
				bestRotated);
	}

	printf("%d won\n",best);
	return best;
}
