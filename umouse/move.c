#include"move.h"
#define NUM_MOVES 7

//fb,rl,turn
struct MOVE MoveNowhere = { 0, 0, STRAIGHT };
struct MOVE MoveStright = { 1, 0, STRAIGHT };
struct MOVE MoveBack = { -1, 0, BACK };
struct MOVE MoveLeft = { 0, -1, LEFT };
struct MOVE MoveRight = { 0, 1, RIGHT };
struct MOVE MoveIntegratedLeft = { 1, -1, LEFT };
struct MOVE MoveIntegratedRight = { 1, 1, RIGHT };

struct MOVE * Moves[NUM_MOVES] = { 
	&MoveNowhere,
	&MoveStright, 
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

struct MOVE * MoveFindBest(INDEX startX, INDEX startY, enum DIRECTION startDir, struct FLOOD_MAP * flood )
{
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

		//apply move and get distance
		MoveApply( &x, &y, &dir, Moves[curMove] );
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
