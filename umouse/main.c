#include"../umouse/map.h"
#include"../umouse/floodfill.h"
#include"../umouse/map.h"
#include"../umouse/positionlog.h"

#include"../kernel/thread.h"

#define WIDTH 16
#define HEIGHT 16

struct MAP Map;
struct FLOOD_MAP FloodMap;
struct SCAN_LOG ScanLog;

FLAG_WORD MouseMapBuff[ MapSizeNeeded(WIDTH,HEIGHT) ];
FLAG_WORD MapBuff[ MapSizeNeeded(WIDTH,HEIGHT) ];
FLAG_WORD FloodMapBuff[ FLOOD_MAP_SIZE(WIDTH,HEIGHT) ];
FLAG_WORD FloodEventBuff[ FLOOD_EVENT_SIZE(WIDTH,HEIGHT) ];
FLAG_WORD ScanLogBuff[ SCAN_LOG_SIZE(WIDTH,HEIGHT) ];

#define STACK_SIZE 300

char WorkerStack[STACK_SIZE];
struct THREAD WorkerThread;

struct THREAD MainThread;
char MainStack[STACK_SIZE];

struct THREAD SensorThread;
char MainStack[STACK_SIZE];

struct THREAD ComputeThread;
char MainStack[STACK_SIZE];

int main()
{
	return 0;
}
