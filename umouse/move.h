#ifndef MOVE_H
#define MOVE_H

#include"../utils/utils.h"
#include"floodfill.h"

struct MOVE 
{
	int Dfb;
	int Drl;
	enum ANGLE Dtheta;
};

extern struct MOVE MoveNowhere;
extern struct MOVE MoveStright;
extern struct MOVE MoveBack;
extern struct MOVE MoveLeft;
extern struct MOVE MoveRight;
extern struct MOVE MoveIntegratedLeft;
extern struct MOVE MoveIntegratedRight;


void MoveApply(INDEX *x, INDEX *y, enum DIRECTION * dir, struct MOVE * move );

struct MOVE * MoveFindBest(INDEX x, INDEX y, enum DIRECTION dir, struct FLOOD_MAP * flood );
#endif
