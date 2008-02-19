#include"move.h"
#include"../utils/utils.h"
#include"submove.h"

#include<stdio.h>

//Stuck Moves
struct MOVE MoveNowhere = { 0, 0, STRAIGHT,
	{SUB_MOVE_DONE} };//dont turn or move
//Single Cell Moves
struct MOVE MoveStraight = { 1, 0, STRAIGHT,
	{	SUB_MOVE_FORWARD_WHOLE, 
		SUB_MOVE_DONE} };//move forward 1 cell
struct MOVE MoveBack = { -1, 0, BACK,
   	{	SUB_MOVE_TURN_AROUND,
		SUB_MOVE_FORWARD_WHOLE,
		SUB_MOVE_DONE} };//turn around and move forward 1 cell//not really needed
struct MOVE MoveLeft = { 0, -1, LEFT,
	{	SUB_MOVE_TURN_LEFT, 
		SUB_MOVE_FORWARD_WHOLE, 
		SUB_MOVE_DONE} };//turn left and move 1 cell
struct MOVE MoveRight = { 0, 1, RIGHT,
	{	SUB_MOVE_TURN_RIGHT, 
		SUB_MOVE_FORWARD_WHOLE, 
		SUB_MOVE_DONE} };//turn right and move 1 cell
//Spin Moves
struct MOVE MoveTurnBack = {0, 0, BACK,
	{	SUB_MOVE_TURN_AROUND,
		SUB_MOVE_DONE} };//turn back
struct MOVE MoveTurnLeft = { 0, 0, LEFT, 
	{	SUB_MOVE_TURN_LEFT, 
		SUB_MOVE_DONE} };//turn left 
struct MOVE MoveTurnRight = { 0, 0, RIGHT,
	{	SUB_MOVE_TURN_RIGHT, 
		SUB_MOVE_DONE} };//turn right
//Integrated Moves
struct MOVE MoveIntegratedLeft = { 1, -1, LEFT,
	{	SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_INTEGRATE_LEFT, 
		SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_DONE} };//turn left while moving forward
struct MOVE MoveIntegratedRight = { 1, 1, RIGHT,
	{	SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_INTEGRATE_RIGHT, 
		SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_DONE} };//turn right while moving forward
//Hairpin Moves
struct MOVE MoveHairpinLeft = { 0, -1, BACK,
   	{	SUB_MOVE_FORWARD_HALF, 
		SUB_MOVE_INTEGRATE_LEFT, 
		SUB_MOVE_INTEGRATE_LEFT,
	   	SUB_MOVE_FORWARD_HALF,
	   	SUB_MOVE_DONE} };
struct MOVE MoveHairpinRight = {0, 1, BACK, 
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


//define funcitons which look for walls
BOOL MoveCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan )
{
	BOOL translated;
	BOOL rotated;
	//assume we are in correct state.
	//iterate through the sub moves checking for wall crossings and scan status
	INDEX moveIndex = 0;

	//sub moves are a 1/2 cell resolution. Always start move from center of cell.
	x=x*2+1;
	y=y*2+1;

	printf("starting move (%d,%d,%d)\n",x,y,dir);

	for( moveIndex = 0; move->SubMoves[moveIndex]!= SUB_MOVE_DONE; moveIndex++ )
	{
		translated = FALSE;
		rotated = FALSE;
		//perform movement
		SubMoveApply(&x,&y,&dir, move->SubMoves[moveIndex],&translated,&rotated);
	
		//perform validation (see if we passed through wall)
		if(	translated && MapGetWall( x/2, y/2, TURN(dir,BACK), map ) )
		{
			printf("hit wall\n");
			return FALSE;
		}
		//make sure we are in scanned cell
		if( translated && ! ScanLogGet( x/2, y/2, scan ) )
		{
			printf("not scanned\n");
			return FALSE;
		}
		//make sure we are not facing a wall(as result of rotation)
		if( rotated&& MapGetWall( x/2, y/2, dir, map ) )
		{
			printf("facing wall\n");
			return FALSE;
		}
	}
	return TRUE;
}

void MoveApply(INDEX * x, INDEX * y, enum DIRECTION * dir, struct MOVE * move )
{
	*x = (*x) *2 +1;
	*y = (*y) *2 +1;
	INDEX index = 0;
	BOOL trans;
	BOOL rot;
	while( move->SubMoves[index] != SUB_MOVE_DONE )
	{
		SubMoveApply(x,y,dir,move->SubMoves[index],&trans,&rot);
		index++;
	}
	*x = *x / 2;
	*y = *y / 2;
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
		validMove = MoveCheck(x,y,dir,Moves[curMove],map,scanLog);

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
		if( ! MoveCheck(nextX, nextY, dir, &MoveStraight, map, scan ) )
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
