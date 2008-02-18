#include"move.h"
#include"../utils/utils.h"
#include"submove.h"

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

BOOL MoveIntegratedCheck(INDEX startX, INDEX startY, enum DIRECTION startDir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan )
{
	INDEX x,y;
	enum DIRECTION dir;
	x = startX;
	y = startY;
	dir = startDir;

	//see if wall in front of us
	if( MapGetWall( x, y, dir, map ) )
		return FALSE;

	//move forward 1 cell
	MoveApply( &x, &y, &dir, &MoveStraight );
	//check if scanned
	if( ! ScanLogGet( x, y, scan ) )
		return FALSE;

	//move into final cell
	MoveApply( &startX, &startY, &startDir, move );

	//check if scanned
	if( ! ScanLogGet( startX, startY, scan ) )
		return FALSE;

	//check for wall behind
	if( MapGetWall( startX, startY, TURN(startDir,BACK), map ) )
		return FALSE;

	//check for wall infront
	if( MapGetWall( startX, startY, startDir, map ) )
		return FALSE;

	return TRUE;
}

BOOL MoveSubMoveCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan )
{
	struct MOVE * turnType;
	//check for wall infront of us
	if( MapGetWall( x, y, dir, map ))
		return FALSE;

	//Move into next cell
	MoveApply( &x, &y, &dir, &MoveStraight );

	//make sure sell is scanned 
	if( ! ScanLogGet( x, y, scan ) )
		return FALSE;

	//figure out turn direction
	if( move->Drl > 0 )//move right
		turnType = &MoveRight;
	else
		turnType = &MoveLeft;

	//move to other side of hairpin
	MoveApply( &x, &y, &dir, turnType );

	//make sure wall is clear between sides of hairpin (behind us)
	if( MapGetWall( x, y, TURN(dir,BACK), map ) )//WRONG
		return FALSE;

	//check to see if cell is checked
	if( ! ScanLogGet( x, y, scan ))
		return FALSE;

	//Move into final cell
	MoveApply( &x, &y, &dir, turnType );

	//check to see if last wall is there (behind us)
	if( MapGetWall( x, y, TURN(dir, BACK), map ) )
		return FALSE;

	//check to see if final cell is scanned.
	if( ! ScanLogGet( x, y, scan ))
		return FALSE;

	//we should not hairpin into a wall
	if( MapGetWall( x, y, dir, map ))
		return FALSE;

	return TRUE;
}

BOOL MoveHairpinCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan )
{
	//assume we are in correct state.
	//iterate through the sub moves checking for wall crossings and scan status
	INDEX moveIndex = 0;

	//sub moves are a 1/2 cell resolution
	x*=2;
	y*=2;

	for( moveIndex = 0; move->SubMoves[moveIndex]!= SUB_MOVE_DONE; moveIndex++ )
	{
		//perform movement
		switch(move->SubMoves[moveIndex])
		{
			case SUB_MOVE_FORWARD_WHOLE:
				SubMoveTranslate(&x,&y,dir,2);
				break;
			case SUB_MOVE_FORWARD_HALF:
				SubMoveTranslate(&x,&y,dir,1);
				break;
			case SUB_MOVE_TURN_RIGHT:
				SubMoveRotate(&dir, RIGHT);
				break;
			case SUB_MOVE_TURN_LEFT:
				SubMoveRotate(&dir, LEFT );
				break;
			case SUB_MOVE_TURN_AROUND:
				SubMoveRotate(&dir, BACK );
				break;
			case SUB_MOVE_INTEGRATE_RIGHT:
				SubMoveTranslate(&x,&y,dir,1);
				SubMoveRotate(&dir, RIGHT);
				break;
			case SUB_MOVE_INTEGRATE_LEFT:
				SubMoveTranslate(&x,&y,dir,1);
				SubMoveRotate(&dir, RIGHT);
				break;
		}
	
		//perform validation (see if we passed through wall)
		if(	MapGetWall( x/2, y/2, dir, map ) )
			return FALSE;
		//make sure we are in scanned cell
		if( ! ScanLogGet( x/2, y/2, scan ) )
			return FALSE;
	}
	return TRUE;
}

//Stuck Moves
struct MOVE MoveNowhere = { 0, 0, STRAIGHT, MoveNoMovementCheck,
	{SUB_MOVE_DONE} };//dont turn or move
//Single Cell Moves
struct MOVE MoveStraight = { 1, 0, STRAIGHT, MoveSingleCheck,
	{	SUB_MOVE_FORWARD_WHOLE, 
		SUB_MOVE_DONE} };//move forward 1 cell
struct MOVE MoveBack = { -1, 0, BACK, MoveSingleCheck,
   	{	SUB_MOVE_TURN_AROUND,
		SUB_MOVE_FORWARD_WHOLE,
		SUB_MOVE_DONE} };//turn around and move forward 1 cell//not really needed
struct MOVE MoveLeft = { 0, -1, LEFT, MoveSingleCheck, 
	{	SUB_MOVE_TURN_LEFT, 
		SUB_MOVE_FORWARD_WHOLE, 
		SUB_MOVE_DONE} };//turn left and move 1 cell
struct MOVE MoveRight = { 0, 1, RIGHT, MoveSingleCheck, 
	{	SUB_MOVE_TURN_RIGHT, 
		SUB_MOVE_FORWARD_WHOLE, 
		SUB_MOVE_DONE} };//turn right and move 1 cell
//Spin Moves
struct MOVE MoveTurnBack = {0, 0, BACK, MoveSpinCheck, 
	{	SUB_MOVE_TURN_AROUND,
		SUB_MOVE_DONE} };//turn back
struct MOVE MoveTurnLeft = { 0, 0, LEFT, MoveSpinCheck, 
	{	SUB_MOVE_TURN_LEFT, 
		SUB_MOVE_DONE} };//turn left 
struct MOVE MoveTurnRight = { 0, 0, RIGHT, MoveSpinCheck, 
	{	SUB_MOVE_TURN_RIGHT, 
		SUB_MOVE_DONE} };//turn right
//Integrated Moves
struct MOVE MoveIntegratedLeft = { 1, -1, LEFT, MoveIntegratedCheck,
	{	SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_INTEGRATE_LEFT, 
		SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_DONE} };//turn left while moving forward
struct MOVE MoveIntegratedRight = { 1, 1, RIGHT, MoveIntegratedCheck, 
	{	SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_INTEGRATE_RIGHT, 
		SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_DONE} };//turn right while moving forward
//Hairpin Moves
struct MOVE MoveHairpinLeft = { 0, -1, BACK, MoveHairpinCheck,
   	{	SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_INTEGRATE_LEFT, 
		SUB_MOVE_INTEGRATE_LEFT,
	   	SUB_MOVE_FORWARD_HALF,
	   	SUB_MOVE_DONE} };
struct MOVE MoveHairpinRight = {0, 1, BACK, MoveHairpinCheck, 
	{	SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_INTEGRATE_RIGHT,
	   	SUB_MOVE_INTEGRATE_RIGHT, 
		SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_DONE} };

#define NUM_MOVES 9
struct MOVE * Moves[] = { 
	&MoveNowhere,//MUST BE FIRST

	&MoveIntegratedLeft,
	&MoveIntegratedRight,
	
	&MoveTurnBack,	
	&MoveTurnRight,
	&MoveTurnLeft,

	&MoveStraight, 
//	&MoveBack, //removed because not clearly needed
//	&MoveLeft, 
//	&MoveRight,

	&MoveHairpinLeft,
	&MoveHairpinRight
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

	//make sure we are not facing a wall to start.
	if( MapGetWall( curX, curY, dir, map ) )
		return 0;

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
