#include"submove.h"

#include<stdio.h>

_Bool SubMoveMovingEndState[9] = 
{false,true,true,false,false,false,false,true,true};

_Bool SubMovesRotates[9] = 
{false,false,false,false,true,true,true,true,true};

char * MoveName[9] = 
{"done","start","forward","stop","turn right","turn left","turn around","int right","int left"};

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
//Complex Move
//

void SubMoveApply(
		INDEX * x, 
		INDEX * y,
	   	enum DIRECTION * dir,
	   	_Bool * moving, 
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
			SubMoveTranslate(x,y,*dir,1);
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
//Test Support Routines
//

#define IS_CENTERED(x,y) ((x)%2 == 1 && (y)%2==1)
#define IS_EDGED(x,y) ((x)%2 == 0 || (y)%2 == 0 )
#define IS_PEGED(x,y) ((x)%2==0 && (y)%2==0)

void GetCell(INDEX *x, INDEX *y, enum DIRECTION dir )
{
	if( IS_CENTERED(*x,*y))
	{
		*x = *x/2;
		*y = *y/2;
	}
	else
	{
		//we are on edge, move into cell.
		SubMoveTranslate( x, y, dir, 1 );
		*x=*x/2;
		*y=*y/2;
	}
}

//
//Test Routines
//

//Tests conditions at start of move
_Bool MoveStartCentered(INDEX x, INDEX y, enum SUB_MOVE move )
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
			return true;
	}
}

_Bool MoveStartEdge( INDEX x, INDEX y, enum SUB_MOVE move )
{
	switch( move )
	{
		case SUB_MOVE_STOP:
		case SUB_MOVE_INTEGRATE_RIGHT:
		case SUB_MOVE_INTEGRATE_LEFT:
			return IS_EDGED(x,y);
		default:
			return true;
	}
}

_Bool MoveStartMoving(_Bool moving, enum SUB_MOVE move )
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
			return true;
	}
}

_Bool MoveStartStraightAway(INDEX startX, INDEX startY, enum DIRECTION dir, enum SUB_MOVE move, struct FLOOD_MAP * flood )
{
	INDEX x;
	INDEX y;
	COUNT curFill;
	COUNT nextFill;
	switch( move )
	{
		case SUB_MOVE_FORWARD:
			x = startX;
			y = startY;

			GetCell( &x, &y, dir );
			//printf("straight away at %d,%d,%d ",startX,startY,dir );
			curFill = FloodFillGet( x, y, flood );
			SubMoveTranslate( &x, &y, dir, 1 );
			nextFill = FloodFillGet( x, y, flood );
			//printf("has flood %d facing flood %d\n",curFill,nextFill);
			if( nextFill < curFill )
				return true;
			else return false;
		default:
			return true;
	}
}

//Tests conditions at end of move
_Bool MoveEndOnWall( INDEX x, INDEX y, struct MAP * map, enum SUB_MOVE move )
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
				return true;//ended in middle of cell, no walls
			else if( x%2 == 1 )
				dir = SOUTH;
			else if( y%2 == 1 )
				dir = WEST;
			else
				return false;//ended on peg, fail
			//printf("end on wall test (%d,%d,%d) == %d\n",
			//		x,y,dir,MapGetWall( x/2, y/2, dir, map));
			return ! MapGetWall( x/2, y/2, dir, map );
		default:
			return true;
	}
}

_Bool MoveEndOnPeg(INDEX x, INDEX y)
{
	if( x%2==0 && y%2==0 )
		return false;
	else
		return true;
}

_Bool MoveEndInScanned(INDEX x, INDEX y, enum DIRECTION dir, struct SCAN_LOG * scan)
{
	GetCell(&x,&y,dir);
	//printf("checking scan in %d,%d = %d\n",x,y,ScanLogGet( x, y, scan ));
	return ScanLogGet( x, y, scan );
}

_Bool MoveNextScanned( 
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
			SubMoveTranslate( &x, &y, dir, 1 );
			GetCell( &x, &y, dir );
			//printf("checking facing scan in %d,%d,%d = %d\n",x,y,dir,ScanLogGet( x, y, scan ));
			return ScanLogGet( x, y, scan );
			break;
		default:
			return true;
	}
}

//Tests conditions in front of mouse (at end of move)
_Bool MoveNextLessFloodFill( INDEX x, INDEX y, enum DIRECTION dir, enum SUB_MOVE move, struct FLOOD_MAP * flood )
{
	COUNT curFill = FloodFillGet( x/2, y/2, flood );//get current cell (end pos)
	SubMoveTranslate( &x, &y, dir, 2 );//look up cordinates for facing cell
	COUNT facingFill = FloodFillGet( x/2, y/2, flood );//get facing flood

	switch( move )
	{
		//case SUB_MOVE_FORWARD:
		case SUB_MOVE_TURN_RIGHT:
		case SUB_MOVE_TURN_LEFT:
		case SUB_MOVE_TURN_AROUND:
			return facingFill < curFill;
			break;
		default:
			return true;
	}
}

_Bool MoveEndFacingWall(INDEX x, INDEX y, enum DIRECTION dir, struct MAP * map, enum SUB_MOVE move )
{
	switch( move )
	{
		case SUB_MOVE_FORWARD:
		case SUB_MOVE_TURN_RIGHT:
		case SUB_MOVE_TURN_LEFT:
		case SUB_MOVE_TURN_AROUND:
		case SUB_MOVE_INTEGRATE_RIGHT:
		case SUB_MOVE_INTEGRATE_LEFT:
			//if we "facing a wall" then we must be in middle
			if ( ! IS_CENTERED(x,y) )
				return true;
			//if we are centered then we can directly look up the wall
			return ! MapGetWall( x/2, y/2, dir, map );
		default:
			return true;

	}
}

//
//Apply all the tests.
//
_Bool SubMoveLegal( 
		INDEX x, 
		INDEX y, 
		enum DIRECTION dir, 
		_Bool moving,
		enum SUB_MOVE move,
		struct MAP *map,
		struct SCAN_LOG *scan,
	   	struct FLOOD_MAP *flood)
{
	//printf("testing %s\n",MoveName[move]);
	//
	//Start Tests
	//
	if( ! MoveStartCentered(x,y,move) )
	{
		//printf("failed centered test\n");
		return false;
	}
	if( ! MoveStartEdge(x,y,move) )
	{
		//printf("failed start edge test\n");
		return false;
	}

	if( !MoveStartMoving( moving, move ) )
	{
		//printf("failed start moving test\n");
		return false;
	}

	if( ! MoveStartStraightAway( x, y, dir, move, flood ))
	{
		//printf("failed straight away test\n");
		return false;
	}

	//
	//End Tests
	//
	
	SubMoveApply( &x, &y, &dir, &moving, move );

	if( ! MoveEndOnWall(x,y,map, move) )
	{
		//printf("failed end on wall test\n");
		return false;
	}
	
	if( ! MoveEndOnPeg(x,y) )
	{
		//printf("ended on peg\n");
		return false;
	}
	if( ! MoveEndInScanned(x,y,dir,scan) )
	{
		//printf("end on scanned test\n");
		return false;
	}
	if( ! MoveEndFacingWall(x,y,dir,map,move) )//NO MOVEMENT
	{
		//printf("failed facing wall test\n");
		return false;
	}
	//
	//Facing Tests
	//
	
	//SubMoveTranslate( &x, &y, dir, 2 );//TODO REFACTOR	
	if( ! MoveNextScanned( x, y, dir, move, scan) )//TRANSLATE 1
	{
		//printf("end facing unexplored cell\n");
		return false;
	}
	if( ! MoveNextLessFloodFill(x,y,dir, move, flood ) )//TRANSLATE 2
	{
		//printf("end not facing less flood\n");
		return false;
	}

	//printf("test %s passed\n",MoveName[move]);
	return true;
}

//
//Selects the next move given current mouse state.
//

enum SUB_MOVE SubMoveFindBest(
		INDEX startX,
		INDEX startY,
		enum DIRECTION startDir,
		_Bool startMoving,
		struct FLOOD_MAP * flood, 
		struct MAP * map,
		struct SCAN_LOG * scan)
{
	//best move vars
	enum SUB_MOVE best;
	unsigned char bestFlood;
	unsigned char bestFacingFlood;
	_Bool bestFacingWall;
	_Bool bestRotated;

	//cur move vars
	enum SUB_MOVE cur;
	unsigned char curFlood;
	unsigned char curFacingFlood;
	_Bool curFacingWall;
	_Bool curRotated;

	//temp calculation vars
	INDEX tempX,tempY;

	//mouse state vars
	INDEX x;
	INDEX y;
	enum DIRECTION dir;
	_Bool moving;

	//set up standard to measure against: SUB_MOVE_DONE
	x = startX;
	y = startY;
	dir = startDir;
	moving = startMoving;
	best = SUB_MOVE_DONE;
	bestFlood = 255;//FloodFillGet( x/2, y/2, flood );
	bestFacingWall = true;//MapGetWall( x/2, y/2, dir, map );
	//SubMoveApply( &x, &y, &dir, &moving, SUB_MOVE_FORWARD );
	bestFacingFlood = 255;//FloodFillGet( x/2, y/2, flood );
	bestRotated = true;//SubMovesRotates[SUB_MOVE_DONE];

	for( cur = SUB_MOVE_START; cur <= SUB_MOVE_INTEGRATE_LEFT; cur++ )
	{
		//gather state vars
		x = startX;
		y = startY;
		dir = startDir;
		moving = startMoving;
		//check move is legal.
		if( ! SubMoveLegal(x, y, dir, moving, cur, map, scan, flood))
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
				//printf("invalid direction for x\n");
				//Check if we have invalid direction for x.
				ASSERT(0);
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
				//printf("invalid direction for y\n");
				//Check if we have invalid direction for y.
				ASSERT(0);
			}
		}
		//printf("normalized corrdinates to %d,%d,%d\n",tempX,tempY,dir);
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
			//printf("lost on flood\n");
			continue;
		}
		else if( curFlood < bestFlood )
			goto comparisonDone;

		//favor facing least floodfill
		if( curFacingFlood > bestFacingFlood)
		{
			//printf("lost on facing flood\n");
			continue;
		}
		else if( curFacingFlood < bestFacingFlood )
			goto comparisonDone;

		//favor not facing wall.
		if( curFacingWall && ! bestFacingWall)
		{
			//printf("lost on facing wall\n");
			continue;
		}
		else if( !curFacingWall && bestFacingWall)
			goto comparisonDone;

		//favor facing same direction
		if( !curRotated && bestRotated )
		{
			//printf("lost on rotation\n");
			continue;
		}
		else if( !bestRotated && curRotated )
			goto comparisonDone;

		//if we tie, keep older
		continue;

comparisonDone:
		//we are better, so take the best vars
		best = cur;
		bestFlood = curFlood;
		bestFacingWall = curFacingWall;
		bestFacingFlood = curFacingFlood;
		bestRotated = curRotated;
		//printf("%s took the lead with ff=%d,facing=%d,fwall=%d,rot=%d\n",
		//		MoveName[best],
		//		bestFlood,
		//		bestFacingFlood,
		//		bestFacingWall,
		//		bestRotated);
	}

	//printf("%s won\n",MoveName[best]);
	return best;
}
