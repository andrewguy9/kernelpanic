#include"move.h"
#include"../utils/utils.h"

#define NUM_MOVES 9

//define funcitons which look for walls
BOOL MoveNoMovementCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan )
{
	return TRUE;
}

BOOL MoveSpinCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan )
{
	//the move just rotates, so simuate rotate and make sure were not facing a wall
	dir = TURN(dir, move->Dtheta);
	return ! MapGetWall( x, y, dir, map );
}

BOOL MoveSingleCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan )
{
	BOOL wall;
	BOOL scanned;
	//we are going to turn and go forward
	//simulate the turn and move
	MoveApply( &x, &y, &dir, move );
	//check see if there is a wall behind us
	dir = TURN( dir, BACK );
	wall = MapGetWall( x, y, dir, map );

	//we move into new cell, make sure we scanned it first
	scanned = ScanLogGet( x, y, scan );

	//move is ok if no wall and scanned
	return !wall && scanned;
}

BOOL MoveIntegratedCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan )
{
	//we are going to go forward 1 cell, check for a wall in that dir
	if( MapGetWall( x, y, dir, map ) || !ScanLogGet(x,y,scan) )
	{
		//there is a wall or not explored
		return FALSE;
	}

	//at the end of the move we will be facing a new direction
	MoveApply( &x, &y, &dir, move );
	//check see if there is a wall behind us or we didn't explore
	dir = TURN( dir, BACK );
	return !MapGetWall( x, y, dir, map ) && ScanLogGet(x,y,scan);
}

//fb,rl,turn,verification function
struct MOVE MoveNowhere = { 0, 0, STRAIGHT, MoveNoMovementCheck };
struct MOVE MoveStraight = { 1, 0, STRAIGHT, MoveSingleCheck };
struct MOVE MoveBack = { -1, 0, BACK, MoveSingleCheck };
struct MOVE MoveLeft = { 0, -1, LEFT, MoveSingleCheck };
struct MOVE MoveRight = { 0, 1, RIGHT, MoveSingleCheck };
struct MOVE MoveTurnLeft = { 0, 0, LEFT, MoveSpinCheck };
struct MOVE MoveTurnRight = { 0, 0, RIGHT, MoveSpinCheck };
struct MOVE MoveIntegratedLeft = { 1, -1, LEFT, MoveIntegratedCheck };
struct MOVE MoveIntegratedRight = { 1, 1, RIGHT, MoveIntegratedCheck };

struct MOVE * Moves[] = { 
	&MoveNowhere,
	&MoveStraight, 
	&MoveBack, 
	&MoveLeft, 
	&MoveRight, 
	&MoveTurnLeft,
	&MoveTurnRight,
	&MoveIntegratedLeft, 
	&MoveIntegratedRight
};

void MoveApply(INDEX * x, INDEX * y, enum DIRECTION * dir, struct MOVE * move )
{
	//calculate our new position
	int dFB = move->Dfb;//forward +, back - 
	int dRL = move->Drl;//left +, right -
	
	//we move relative to start dir.
	switch( * dir )
	{
		case NORTH:
			//we facing north, so y = dFB and x = dRL
			*x+=dRL;
			*y+=dFB;
			break;
		case SOUTH:
			//we are facing south, so x=-dRL and y = -dFB
			*x-=dRL;
			*y-=dFB;
			break;
		case EAST:
			//we are facing east, so x = dFB, y = -dRL
			*x+=dFB;
			*y-=dRL;
			break;
		case WEST:
			//we are facing west, so x = -dFB, y = dRL
			*x-=dFB;
			*y+=dRL;
			break;
	}

	//calculate our final orientation
	*dir = TURN( *dir, move->Dtheta );
}

struct MOVE * MoveFindBest(
		INDEX startX,
	   	INDEX startY,
	   	enum DIRECTION startDir,
	   	struct FLOOD_MAP * flood, 
		struct MAP * map,
		struct SCAN_LOG * scanLog)
{
	BOOL validMove;
	INDEX bestMove = 0;
	INDEX curMove;

	INDEX x,y,dir;
	//do move nowhere.
	unsigned char curDist;
	unsigned char bestDist = FloodFillGet( x, y, flood );

	for(curMove = 1; curMove < NUM_MOVES; curMove ++ )
	{
		//set conditions
		x = startX;
		y = startY;
		dir = startDir;

		//test to see if move is valid (dont cross walls)
		validMove = Moves[curMove]->Check(x,y,dir,Moves[curMove],map,scanLog);

		if( !validMove)
			continue;

		//apply move to final position
		MoveApply( &x, &y, &dir, Moves[curMove] );

		//get the distance
		curDist = FloodFillGet( x, y, flood );

		//check for improvement
		if( curDist < bestDist )
		{//this move is better than our current best, switch
			bestDist = curDist;
			bestMove = curMove;
		}
	}
	return Moves[bestMove];
}
