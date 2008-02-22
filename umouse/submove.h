#ifndef SUB_MOVE_H
#define SUB_MOVE_H

#include"compass.h"
#include"../utils/utils.h"
#include"positionlog.h"
#include"map.h"
#include"floodfill.h"

enum SUB_MOVE { 
	SUB_MOVE_DONE,
   	SUB_MOVE_START,//1
	SUB_MOVE_FORWARD,//2
	SUB_MOVE_STOP,//3
	SUB_MOVE_TURN_RIGHT,//4
	SUB_MOVE_TURN_LEFT,//5
	SUB_MOVE_TURN_AROUND,//6
	SUB_MOVE_INTEGRATE_RIGHT,//7
	SUB_MOVE_INTEGRATE_LEFT//8
};

void SubMoveTranslate( INDEX * x, INDEX * y, enum DIRECTION dir, COUNT dist );

void SubMoveRotate( enum DIRECTION * dir, enum ANGLE angle );

void SubMoveApply(INDEX * x, INDEX * y, enum DIRECTION * dir, BOOL * moving, enum SUB_MOVE move);

BOOL SubMoveLegal( 
		INDEX x, 
		INDEX y, 
		enum DIRECTION startDir, 
		BOOL moving,
		enum SUB_MOVE move,
		struct MAP *map,
	   	struct SCAN_LOG *scan,
		struct FLOOD_MAP * flood);

enum SUB_MOVE SubMoveFindBest(
		INDEX startX,
	   	INDEX startY,
	   	enum DIRECTION startDir,
		BOOL moving,
	   	struct FLOOD_MAP * flood, 
		struct MAP * map,
		struct SCAN_LOG * scanLog);
#endif
