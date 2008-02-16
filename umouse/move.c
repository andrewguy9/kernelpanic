#include"move.h"
#include"../utils/utils.h"

#define NUM_MOVES 5

//define funcitons which look for walls
BOOL MoveNoMovementCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map )
{
	return TRUE;
}

BOOL MoveSingleCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map )
{
	//we are going to turn and go forward
	//simulate the turn and see if a wall faces you
	MoveApply( &x, &y, &dir, move );
	//check see if there is a wall behind us
	dir = TURN( dir, BACK );
	return !MapGetWall( x, y, dir, map );
}

BOOL MoveIntegratedCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map )
{
	//we are going to go forward 1 cell, check for a wall in that dir
	if( MapGetWall( x, y, dir, map ) )
	{
		//there is a wall, fail move
		return FALSE;
	}

	//at the end of the move we will be facing a new direction
	MoveApply( &x, &y, &dir, move );
	//check see if there is a wall behind us
	dir = TURN( dir, BACK );
	return !MapGetWall( x, y, dir, map );
}

//fb,rl,turn
struct MOVE MoveNowhere = { 0, 0, STRAIGHT, MoveNoMovementCheck };
struct MOVE MoveStraight = { 1, 0, STRAIGHT, MoveSingleCheck };
struct MOVE MoveBack = { -1, 0, BACK, MoveSingleCheck };
struct MOVE MoveLeft = { 0, -1, LEFT, MoveSingleCheck };
struct MOVE MoveRight = { 0, 1, RIGHT, MoveSingleCheck };
struct MOVE MoveIntegratedLeft = { 1, -1, LEFT, MoveIntegratedCheck };
struct MOVE MoveIntegratedRight = { 1, 1, RIGHT, MoveIntegratedCheck };

struct MOVE * Moves[NUM_MOVES] = { 
	&MoveNowhere,
	&MoveStraight, 
	&MoveBack, 
	&MoveLeft, 
	&MoveRight, 
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

struct MOVE * MoveFindBest(INDEX startX, INDEX startY, enum DIRECTION startDir, struct FLOOD_MAP * flood, struct MAP * map )
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
		validMove = Moves[curMove]->Check(x,y,dir,Moves[curMove],map);

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
