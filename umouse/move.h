#ifndef MOVE_H
#define MOVE_H

#include"../utils/utils.h"
#include"floodfill.h"
#include"compass.h"
#include"positionlog.h"
#include"submove.h"


struct MOVE 
{
	enum SUB_MOVE SubMoves[5];
};


BOOL MoveCheck(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map, struct SCAN_LOG *scan );

extern struct MOVE MoveNowhere;
extern struct MOVE MoveStraight;
extern struct MOVE MoveBack;
extern struct MOVE MoveLeft;
extern struct MOVE MoveRight;
extern struct MOVE MoveTurnBack;
extern struct MOVE MoveTurnLeft;
extern struct MOVE MoveTurnRight;
extern struct MOVE MoveIntegratedLeft;
extern struct MOVE MoveIntegratedRight;
extern struct MOVE MoveHairpinLeft;
extern struct MOVE MoveHairpinRight;

//Applies a move to mouse state variables x, y and dir.
void MoveApply(INDEX *x, INDEX *y, enum DIRECTION * dir, struct MOVE * move );

//Finds the best move given an up to date floodmap and map at position startx, starty and startdir. 
struct MOVE * MoveFindBest(
		INDEX startX, 
		INDEX startY, 
		enum DIRECTION startDir, 
		struct FLOOD_MAP * flood, 
		struct MAP * map,
	   	struct SCAN_LOG * scanLog);

COUNT MoveStraightAwayLength(
		INDEX startX,
		INDEX startY,
		enum DIRECTION dir,
		struct MAP * map,
		struct FLOOD_MAP * floodMap,
		struct SCAN_LOG * scan);
#endif
