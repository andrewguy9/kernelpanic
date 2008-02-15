#ifndef MOVE_H
#define MOVE_H

#include"../utils/utils.h"
#include"floodfill.h"
struct MOVE;

typedef BOOL (WALLS_OK_FUNCTION)(INDEX x, INDEX y, enum DIRECTION dir, struct MOVE * move, struct MAP * map );

struct MOVE 
{
	int Dfb;
	int Drl;
	enum ANGLE Dtheta;
	WALLS_OK_FUNCTION * Check;
};

extern struct MOVE MoveNowhere;
extern struct MOVE MoveStright;
extern struct MOVE MoveBack;
extern struct MOVE MoveLeft;
extern struct MOVE MoveRight;
extern struct MOVE MoveIntegratedLeft;
extern struct MOVE MoveIntegratedRight;


void MoveApply(INDEX *x, INDEX *y, enum DIRECTION * dir, struct MOVE * move );

struct MOVE * MoveFindBest(INDEX startX, INDEX startY, enum DIRECTION startDir, struct FLOOD_MAP * flood, struct MAP * map );
#endif
