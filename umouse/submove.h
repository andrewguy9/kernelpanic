#ifndef SUB_MOVE_H
#define SUB_MOVE_H

#include"compass.h"
#include"../utils/utils.h"

enum SUB_MOVE { 
	SUB_MOVE_DONE, 
	SUB_MOVE_FORWARD_WHOLE, 
	SUB_MOVE_FORWARD_HALF,
	SUB_MOVE_TURN_RIGHT,
	SUB_MOVE_TURN_LEFT,
	SUB_MOVE_TURN_AROUND,
	SUB_MOVE_INTEGRATE_RIGHT,
	SUB_MOVE_INTEGRATE_LEFT
};

void SubMoveTranslate( INDEX * x, INDEX * y, enum DIRECTION dir, COUNT dist );

void SubMoveRotate( enum DIRECTION * dir, enum ANGLE angle );

void SubMoveApply(INDEX * x, INDEX * y, enum DIRECTION * dir, enum SUB_MOVE move, BOOL * translated, BOOL * rotated );
#endif
