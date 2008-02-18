#include"move.h"
#include"../utils/utils.h"

//define funcitons which look for walls
BOOL MoveNoMovementCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan )
{
	return TRUE;
}

BOOL MoveSpinCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan )
{
	//the move just rotates, so simuate rotate and 
	//make sure were not facing a wall
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

//Stuck Moves
struct MOVE MoveNowhere = { 0, 0, STRAIGHT, MoveNoMovementCheck };//dont turn or move
//Single Cell Moves
struct MOVE MoveStraight = { 1, 0, STRAIGHT, MoveSingleCheck };//move forward 1 cell
struct MOVE MoveBack = { -1, 0, BACK, MoveSingleCheck };//turn around and move forward 1 cell
struct MOVE MoveLeft = { 0, -1, LEFT, MoveSingleCheck };//turn left and move 1 cell
struct MOVE MoveRight = { 0, 1, RIGHT, MoveSingleCheck };//turn right and move 1 cell
//Spin Moves
struct MOVE MoveTurnBack = {0, 0, BACK, MoveSpinCheck};//turn back
struct MOVE MoveTurnLeft = { 0, 0, LEFT, MoveSpinCheck };//turn left 
struct MOVE MoveTurnRight = { 0, 0, RIGHT, MoveSpinCheck };//turn right
//Integrated Moves
struct MOVE MoveIntegratedLeft = { 1, -1, LEFT, MoveIntegratedCheck };//turn left while moving forward
struct MOVE MoveIntegratedRight = { 1, 1, RIGHT, MoveIntegratedCheck };//turn right while moving forward

#define NUM_MOVES 10
struct MOVE * Moves[] = { 
	&MoveNowhere,//MUST BE FIRST

	&MoveIntegratedLeft,
	&MoveIntegratedRight,
	
	&MoveTurnBack,	
	&MoveTurnRight,
	&MoveTurnLeft,

	&MoveStraight, 
	&MoveBack, 
	&MoveLeft, 
	&MoveRight,
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
	BOOL validMove;//wheather the move is legal
	INDEX bestMove = 0; //index of best move so far.
	INDEX curMove;//index of current move.

	INDEX x,y,dir;//position of mouse
	INDEX nextX, nextY, nextDir;//position of cell in front of mouse

	unsigned char curDist;//dist of current move
	unsigned char curNextDist;//dist of cell in from of current move.
	
	//do move nowhere (for reference.)
	unsigned char bestDist = FloodFillGet( x, y, flood );
	nextX = x;
	nextY = y;
	nextDir = dir;
	MoveApply( &nextX, &nextY, &nextDir, &MoveStraight );
	unsigned char bestNextDist = FloodFillGet( nextX, nextY, flood );


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

		//get the distance to destination
		curDist = FloodFillGet( x, y, flood );

		//get the distance from the cell we are facing to the desination
		nextX = x;
		nextY = y;
		nextDir = dir;
		MoveApply( &nextX, &nextY, &nextDir, &MoveStraight );
		curNextDist = FloodFillGet( nextX, nextY, flood );

		//check for improvement
		if( (curDist < bestDist) || 
				(curDist == bestDist && curNextDist < bestNextDist) ||
		 		(curDist == bestDist && curNextDist == bestNextDist && dir == startDir))
		{
			//this move is better than our current best, switch:
			//better is defined as (in decending importance)
			//	has lower floodfill, 
			//	is facing a cell with lower floodfill, 
			//	is facing same direction as we started.
			bestDist = curDist;
			bestNextDist = curNextDist;
			bestMove = curMove;
		}
	}
	return Moves[bestMove];
}

COUNT MoveStraightAwayLength(
		INDEX curX,
		INDEX curY,
		enum DIRECTION dir,
		struct MAP * map,
		struct FLOOD_MAP * flood,
		struct SCAN_LOG * scan)
{
	INDEX nextX;
	INDEX nextY;

	//straight aways are corridors which have 
	//decending floodfill, cross no walls and 
	//consist only of scanned cells.
	COUNT numSteps = 0;
	unsigned char curFlood;
	unsigned char nextFlood;

	//get start flood for reference
	curFlood = FloodFillGet( curX, curY, flood );
	do
	{
		nextX = curX;
		nextY = curY;

		//get cordninates of next cell
		MoveApply( &nextX, &nextY , &dir , &MoveStraight );

		//check the move is actually valid
		if( ! MoveSingleCheck(nextX, nextY, dir, &MoveStraight, map, scan) )
			break;//move not legal, must have crossed a wall or non scanned cell

		//check floodfill dropping
		nextFlood = FloodFillGet( nextX, nextY, flood );
		if( nextFlood >= curFlood )
			break;//no improvement

		//move is success, commit to move
		curX = nextX;
		curY = nextY;
		curFlood = nextFlood;
		numSteps++;
		
	}while(TRUE);

	return numSteps;
}
