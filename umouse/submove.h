#ifndef SUB_MOVE_H
#define SUB_MOVE_H

#include"compass.h"
#include"../utils/utils.h"
#include"positionlog.h"
#include"map.h"
#include"floodfill.h"

enum SUB_MOVE { 
	SUB_MOVE_DONE,
   	SUB_MOVE_START,
	SUB_MOVE_STOP,	
	SUB_MOVE_FORWARD,
	SUB_MOVE_TURN_RIGHT,
	SUB_MOVE_TURN_LEFT,
	SUB_MOVE_TURN_AROUND,
	SUB_MOVE_INTEGRATE_RIGHT,
	SUB_MOVE_INTEGRATE_LEFT
};

char SubMoveTranslate( INDEX * x, INDEX * y, enum DIRECTION dir, COUNT dist );

char SubMoveRotate( enum DIRECTION * dir, enum ANGLE angle );

char SubMoveApply(INDEX * x, INDEX * y, enum DIRECTION * dir, BOOL * moving, enum SUB_MOVE move);

BOOL SubMoveLegal( 
		INDEX x, 
		INDEX y, 
		enum DIRECTION startDir, 
		BOOL moving,
		enum SUB_MOVE move,
		struct MAP *map,
	   	struct SCAN_LOG *scan );

enum SUB_MOVE SubMoveFindBest(
		INDEX startX,
	   	INDEX startY,
	   	enum DIRECTION startDir,
		BOOL moving,
	   	struct FLOOD_MAP * flood, 
		struct MAP * map,
		struct SCAN_LOG * scanLog);
#endif
