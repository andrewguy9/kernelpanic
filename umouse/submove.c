#include"submove.h"

#include<stdio.h>

BOOL SubMoveMovingEndState[9] = 
{FALSE,TRUE,FALSE,TRUE,FALSE,FALSE,FALSE,TRUE,TRUE};

BOOL SubMovesRotates[9] = 
{FALSE,FALSE,FALSE,FALSE,TRUE,TRUE,TRUE,TRUE,TRUE};

char * MoveName[9] = 
{"done","start","stop","forward","turn right","turn left","turn around","int right","int left"};
//
//Primative Move Functions
//

void SubMoveTranslate( 
		INDEX * x, 
		INDEX * y, 
		enum DIRECTION dir, 
		COUNT dist )
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
	
void SubMoveRotate( 
		enum DIRECTION * dir, 
		enum ANGLE angle )
{
	* dir = TURN((*dir),angle);
}

//
//Apply a complete move
//

void SubMoveApply(
		INDEX * x, 
		INDEX * y,
	   	enum DIRECTION * dir,
	   	BOOL * moving, 
		enum SUB_MOVE move)
{
	*moving = SubMoveMovingEndState[move];
	switch(move)
	{
		case SUB_MOVE_DONE:
			break;
		case SUB_MOVE_START:
			SubMoveTranslate(x,y,*dir,1);
			//printf("forward start = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_STOP:
			SubMoveTranslate( x, y, *dir, 1 );
			//printf("forward stop = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_FORWARD:
			SubMoveTranslate(x,y,*dir,2);
			break;
		case SUB_MOVE_TURN_RIGHT:
			SubMoveRotate(dir, RIGHT);
			//printf("turn right = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_TURN_LEFT:
			SubMoveRotate(dir, LEFT);
			//printf("turn left = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_TURN_AROUND:
			SubMoveRotate(dir, BACK );
			//printf("turn around = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_INTEGRATE_RIGHT:
			SubMoveTranslate(x,y,*dir,1);
			SubMoveRotate(dir, RIGHT);
			SubMoveTranslate( x, y, *dir, 1 );
			//printf("integrate right = %d,%d,%d\n",*x,*y,*dir);
			break;
		case SUB_MOVE_INTEGRATE_LEFT:
			SubMoveTranslate(x,y,*dir,1);
			SubMoveRotate(dir, LEFT);
			SubMoveTranslate( x, y, *dir, 1 );
			//printf("integrate left = %d,%d,%d\n",*x,*y,*dir);
			break;
	}
}

//
//Individial tests
//

#define IS_CENTERED(x,y) (!((x)%2 == 0 && (y)%2==0))
#define IS_EDGED(x,y) ((x)%2 == 1 || (y)%2==1)
#define IS_PEGED(x,y) ((x)%2==1 && (y)%2==1)
BOOL MoveStartCentered(INDEX x, INDEX y, enum SUB_MOVE move )
{
	switch( move )
	{
		case SUB_MOVE_START:
		case SUB_MOVE_TURN_RIGHT:
		case SUB_MOVE_TURN_LEFT:
		case SUB_MOVE_TURN_AROUND:
			return IS_CENTERED(x,y);
			break;
		default:
			return TRUE;
	}
}

BOOL MoveStartEdge( INDEX x, INDEX y, enum SUB_MOVE move )
{
	switch( move )
	{
		case SUB_MOVE_FORWARD:
		case SUB_MOVE_STOP:
		case SUB_MOVE_INTEGRATE_RIGHT:
		case SUB_MOVE_INTEGRATE_LEFT:
			return IS_EDGED(x,y);
		default:
			return TRUE;
	}
}

BOOL MoveStartMoving(BOOL moving, enum SUB_MOVE move )
{
	switch( move )
	{
		//need to be not moving at start
		case SUB_MOVE_START:
		case SUB_MOVE_TURN_RIGHT:
		case SUB_MOVE_TURN_LEFT:
		case SUB_MOVE_TURN_AROUND:
			return ! moving;
			break;
		//need to be moving
		case SUB_MOVE_STOP:
		case SUB_MOVE_FORWARD:
		case SUB_MOVE_INTEGRATE_RIGHT:
		case SUB_MOVE_INTEGRATE_LEFT:
			return moving;
			break;
		//dont care
		default:
			return TRUE;
	}
}

BOOL MoveEndOnWall( INDEX x, INDEX y, struct MAP * map, enum SUB_MOVE move )
{
	//TODO BROKEN
	enum DIRECTION dir;
	switch( move )
	{
		case SUB_MOVE_START:
		case SUB_MOVE_STOP:
		case SUB_MOVE_FORWARD:
		case SUB_MOVE_INTEGRATE_RIGHT:
		case SUB_MOVE_INTEGRATE_LEFT:
			if(x%2==1 && y%2==1)
				return TRUE;//ended in middle of cell, no walls
			else if( x%2 == 1 )
				dir = SOUTH;
			else if( y%2 == 1 )
				dir = WEST;
			else
				return FALSE;//ended on peg, fail
			printf("end on wall test (%d,%d,%d) == %d\n",
					x,y,dir,MapGetWall( x/2, y/2, dir, map));
			return ! MapGetWall( x/2, y/2, dir, map );
		default:
			return TRUE;
	}
}

BOOL MoveEndFacingWall(INDEX x, INDEX y, enum DIRECTION dir, struct MAP * map, enum SUB_MOVE move )
{
	switch( move )
	{
		case SUB_MOVE_FORWARD:
		case SUB_MOVE_TURN_RIGHT:
		case SUB_MOVE_TURN_LEFT:
		case SUB_MOVE_TURN_AROUND:
		case SUB_MOVE_INTEGRATE_RIGHT:
		case SUB_MOVE_INTEGRATE_LEFT:
			x=x/2;
			y=y/2;
			return ! MapGetWall( x, y, dir, map );
		default:
			return TRUE;

	}
}

BOOL MoveEndOnPeg(INDEX x, INDEX y)
{
	if( x%2==0 && y%2==0 )
		return FALSE;
	else
		return TRUE;
}

BOOL MoveEndInScanned(INDEX x, INDEX y, struct SCAN_LOG * scan)
{
	return ScanLogGet( x/2, y/2, scan );
}

BOOL MoveEndFacingScanned( 
		INDEX x, 
		INDEX y, 
		enum DIRECTION dir,
		enum SUB_MOVE move, 
		struct SCAN_LOG * scan )
{
	switch( move )
	{
		case SUB_MOVE_FORWARD:
		case SUB_MOVE_INTEGRATE_RIGHT:
		case SUB_MOVE_INTEGRATE_LEFT:
			SubMoveTranslate( &x, &y, dir, 2 );
			return ScanLogGet( x/2, y/2, scan );
			break;
		default:
			return TRUE;
	}
}
//
//Apply all the tests.
//
BOOL SubMoveLegal( 
		INDEX x, 
		INDEX y, 
		enum DIRECTION dir, 
		BOOL moving,
		enum SUB_MOVE move,
		struct MAP *map,
		struct SCAN_LOG *scan )
{
	printf("testing %s\n",MoveName[move]);
	if( ! MoveStartCentered(x,y,move) )
	{
		printf("failed centered test\n");
		return FALSE;
	}
	if( ! MoveStartEdge(x,y,move) )
	{
		printf("failed start edge test\n");
		return FALSE;
	}

	if( !MoveStartMoving( moving, move ) )
	{
		printf("failed start moving test\n");
		return FALSE;
	}

	SubMoveApply( &x, &y, &dir, &moving, move );

	if( ! MoveEndOnWall(x,y,map, move) )
	{
		printf("failed end on wall test\n");
		return FALSE;
	}
	if( ! MoveEndFacingWall(x,y,dir,map,move) )
	{
		printf("failed facing wall test\n");
		return FALSE;
	}
	if( ! MoveEndOnPeg(x,y) )
	{
		printf("ended on peg\n");
		return FALSE;
	}
	if( ! MoveEndInScanned(x,y,scan) )
	{
		printf("end on scanned test\n");
		return FALSE;
	}
	if( ! MoveEndFacingScanned( x, y, dir, move, scan) )
	{
		printf("end facing unexplored cell\n");
		return FALSE;
	}

	return TRUE;
}

//
//
//

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

	//temp calculation vars
	INDEX tempX,tempY;

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

		//determine which cell we are "in"
		tempX = x;
		tempY = y;
		if( x%2==1 )
			tempX = x/2;
		else 
		{
			if( dir == EAST )
				tempX = x/2;
			else if( dir == WEST )
				tempX = x/2-1;
			else
			{
				printf("invalid direction for x\n");
				exit(0);
			}
		}
		if( y%2==1 )
			tempY = y/2;
		else 
		{
			if( dir == NORTH )
				tempY = y/2;
			else if( dir == SOUTH )
				tempY = y/2-1;
			else
			{
				printf("invalid direction for y\n");
				exit(0);
			}
		}
		printf("normalized corrdinates to %d,%d,%d\n",tempX,tempY,dir);
		//gather results
		curFlood = FloodFillGet( tempX, tempY, flood );
		curFacingWall = MapGetWall( tempX, tempY, dir, map );
		//translate for scan lookup
		SubMoveTranslate( &tempX, &tempY, dir, 1 );
		curFacingFlood = FloodFillGet( tempX, tempY, flood );
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

	printf("%s won\n",MoveName[best]);
	return best;
}
