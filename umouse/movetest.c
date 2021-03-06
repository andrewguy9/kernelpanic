#include"submove.h"
#include"map.h"
#include"floodfill.h"
#include"positionlog.h"

#include<stdio.h>
#include<stdlib.h>

#define WIDTH 8
#define HEIGHT 8

void PrintMapFlood( struct MAP * map, struct FLOOD_MAP * flood, INDEX mouseX, INDEX mouseY, enum DIRECTION mouseDir)
{
	INDEX x, y;

	//construct a line
	for( y = map->Height; y!=-1; y--)
	{
		//construct cells (EAST and WEST walls)
		if( y != map->Height)//dont for top row
		{
			//scan left to right.
			for(x=0; x<map->Width; x++)
			{
				//western wall
				if( MapGetWall( x, y, WEST, map ) )
					printf("|");
				else if( mouseX/2 == x && mouseY/2 == y && mouseX%2==0 && mouseY%2==0 )
					printf("M");
				else
					printf(" ");
				//Print cell
				if( mouseX == x && mouseY == y )
				{//print mouse pos and dir
					switch(mouseDir)
					{
						case NORTH:
							printf("/\\ ");
							break;
						case SOUTH:
							printf("\\/ ");
							break;
						case EAST:
							printf(" > ");
							break;
						case WEST:
							printf(" < ");
							break;
					}
				}
				else
				{//print flood
					printf("%3x",FloodFillGet( x, y, flood));
				}
			}
			//wall
			if( MapGetWall( x, y, WEST, map ) )
				printf("|");
			else
				printf(" ");
			//end the line
			printf("\n");
		}
		//construct southern wall
		for( x=0; x<map->Width; x++)
		{
			printf("+");//pin
			if( MapGetWall(x, y, SOUTH, map ) )
				printf("---");//wall
			else
				printf("   ");
		}
		printf("+");//east pin
		printf("\n");
	}
}

void PrintMapScan( struct MAP * map, struct SCAN_LOG * scan, INDEX mouseX, INDEX mouseY, enum DIRECTION mouseDir)
{
	INDEX x, y;

	//construct a line
	for( y = map->Height; y!=-1; y--)
	{
		//construct cells (EAST and WEST walls)
		if( y != map->Height)//dont for top row
		{
			//scan left to right.
			for(x=0; x<map->Width; x++)
			{
				//western wall
				if( MapGetWall( x, y, WEST, map ) )
					printf("|");
				else
					printf(" ");
				//Print cell
				if( mouseX == x && mouseY == y )
				{//print mouse pos and dir
					switch(mouseDir)
					{
						case NORTH:
							ScanLogGet( x, y, scan ) ? printf("/S\\") : printf("/\\ ");
							break;
						case SOUTH:
							ScanLogGet( x, y, scan ) ? printf("\\S/") :	printf("\\/ ");
							break;
						case EAST:
							ScanLogGet( x, y, scan ) ? printf("-S>") : printf(" > ");
							break;
						case WEST:
							ScanLogGet( x, y, scan ) ? printf("<S-") : printf(" < ");
							break;
					}
				}
				else
				{//print flood
					if(ScanLogGet( x, y, scan))
						printf(" S ");
					else
						printf("   ");
				}
			}
			//wall
			if( MapGetWall( x, y, WEST, map ) )
				printf("|");
			else
				printf(" ");
			//end the line
			printf("\n");
		}
		//construct southern wall
		for( x=0; x<map->Width; x++)
		{
			printf("+");//pin
			if( MapGetWall(x, y, SOUTH, map ) )
				printf("---");//wall
			else
				printf("   ");
		}
		printf("+");//east pin
		printf("\n");
	}
}

void MakeLines(INDEX x, INDEX y, COUNT dist, enum DIRECTION moveDir, enum DIRECTION wallDir, struct MAP * map )
{
	for (; dist > 0; dist--) {
		MapSetWall( x, y, wallDir, true, map );
		SubMoveTranslate( &x, &y, moveDir, 1 );
	}
}

struct MAP MouseMap;
struct MAP WorldMap;
struct FLOOD_MAP FloodMap;
struct SCAN_LOG ScanLog;

BITMAP_WORD MouseMapBuff[ MapSizeNeeded(WIDTH,HEIGHT) ];
BITMAP_WORD WorldMapBuff[ MapSizeNeeded(WIDTH,HEIGHT) ];
BITMAP_WORD FloodMapBuff[ FLOOD_MAP_SIZE(WIDTH,HEIGHT) ];
BITMAP_WORD FloodEventBuff[ FLOOD_EVENT_SIZE(WIDTH,HEIGHT) ];
BITMAP_WORD ScanLogBuff[ SCAN_LOG_SIZE(WIDTH,HEIGHT) ];

void PrintMove( enum SUB_MOVE move )
{
	switch( move )
	{
		case SUB_MOVE_START:
			printf("start");
			break;
		case SUB_MOVE_STOP:
			printf("stop");
			break;
		case SUB_MOVE_FORWARD:
			printf("forward");
			break;
		case SUB_MOVE_TURN_RIGHT:
			printf("turn right");
			break;
		case SUB_MOVE_TURN_LEFT:
			printf("turn left");
			break;
		case SUB_MOVE_TURN_AROUND:
			printf("turn around");
			break;
		case SUB_MOVE_INTEGRATE_RIGHT:
			printf("integrate right");
			break;
		case SUB_MOVE_INTEGRATE_LEFT:
			printf("integrate left");
			break;
		case SUB_MOVE_DONE:
		default:
		printf("NOT POSSIBLE MOVE\n");
		exit(0);
	}
}

void PrintState(INDEX x, INDEX y, enum DIRECTION dir, _Bool moving )
{
	printf("(%ld.%ld,%ld.%ld) ",x/2,x%2,y/2,y%2);
	switch(dir)
	{
		case NORTH:
			printf("n ");
			break;
		case SOUTH:
			printf("s ");
			break;
		case EAST:
			printf("e ");
			break;
		case WEST:
			printf("w ");
			break;
	}
	if( moving )
		printf("moving ");
	else
		printf("not moving ");

	printf("\n");
}

void UpdateMap( INDEX x, INDEX y, enum DIRECTION dir, _Bool moving, INDEX destX, INDEX destY)
{
	INDEX scanX, scanY;
	enum DIRECTION testDir;

	_Bool mapChanged = false;

	scanX = x;
	scanY = y;

	if( x%2 == 0 || y%2== 0 )
	{
		printf("cant scan, not in middle\n");
		goto done;
	}

	//see if there is a wall infront of us
	if( ! MapGetWall( x/2, y/2, dir, &WorldMap ))
	{
		printf("scanning\n");
		//move cordinates into next cell.
		SubMoveTranslate( &scanX, &scanY, dir, 2 );
		//no wall so we can scan.
		//check walls for changes to update floodfill
		for(testDir = NORTH; testDir <= WEST; testDir++)
		{
			if( ! MapGetWall( scanX/2, scanY/2, testDir, &MouseMap ) &&
					MapGetWall( scanX/2, scanY/2, testDir, &WorldMap ) )
			{//mouse just saw new wall
				MapSetWall( scanX/2, scanY/2, testDir, true, &MouseMap );
				mapChanged = true;
			}
		}
		//Log that we scanned the cell.
		ScanLogSet(scanX/2,scanY/2,true,&ScanLog);
	}
	else
	{
		printf("cant scan, wall too close\n");
	}

	if( mapChanged )
	{
		printf( "found new walls at (%ld.%ld,%ld.%ld)\n", scanX/2,scanX%2, scanY/2,scanY%2 );
		FloodFillClear( &FloodMap );
		FloodFillSetDestination( destX, destY, &FloodMap );
		FloodFillCalculate( &MouseMap, &FloodMap );
	}
done:
	printf("world map\n");
	PrintMapScan( &WorldMap, &ScanLog, x/2, y/2, dir );
	printf("mouse map\n");
	PrintMapFlood( &MouseMap, &FloodMap, x/2, y/2, dir );
}

void RunMoves(INDEX * startX, INDEX *startY, enum DIRECTION *startDir, INDEX destX, INDEX destY )
{
	//Mouse state variables.
	INDEX x= *startX, y= *startY;
	enum DIRECTION dir = *startDir;
	enum SUB_MOVE move = SUB_MOVE_DONE;
	_Bool moving = false;

	//force a flood fill calculation
	FloodFillClear( &FloodMap );
	FloodFillSetDestination( destX, destY, &FloodMap );
	FloodFillCalculate( &MouseMap, &FloodMap );

	//adjust resolution
	x = x*2+1;
	y = y*2+1;

	//start makeing moves
	do
	{
		printf("------------------------------------------------------\n");

		//Map the area infront of you
		UpdateMap( x, y, dir, moving, destX, destY );

		//Print the state
		printf("dist = %d ", FloodFillGet( x/2, y/2, &FloodMap));
		PrintState( x, y, dir,moving );
		if( x > WIDTH*2 || y > HEIGHT*2 )
		{
			printf("out of bounds\n");
			exit(0);
		}

		//Make the decision
		move = SubMoveFindBest( x, y, dir, moving, &FloodMap, &MouseMap, &ScanLog );
		PrintMove( move );

		//Make the move
		SubMoveApply( &x, &y, &dir, &moving, move);

		printf("now at ");
		PrintState(x,y,dir,moving);

		printf("dist = %d \n", FloodFillGet( x, y, &FloodMap));

	}while( FloodFillGet( x/2, y/2, &FloodMap) != 0 );

	printf("Ended up at (%ld.%ld,%ld.%ld)\n",x/2,x%2,y/2,y%2);
	printf("world map\n");
	PrintMapFlood( &WorldMap, &FloodMap, x/2, y/2, dir );
	printf("mouse map\n");
	PrintMapFlood( &MouseMap, &FloodMap, x/2, y/2, dir );

	//move resolution back
	if(x %2 == 0 || y%2 == 0)
	{
		printf("didn't end up in center\n");
		exit(0);
	}
	x = x/2;
	y = y/2;

	//output back to main
	*startX = x;
	*startY = y;
	*startDir = dir;
}

int main()
{
	//Initialize Maps
	MapInit( &WorldMap, WorldMapBuff, MapSizeNeeded(WIDTH,HEIGHT), WIDTH, HEIGHT );
	MapInit( &MouseMap, MouseMapBuff, MapSizeNeeded(WIDTH,HEIGHT), WIDTH, HEIGHT );
	ScanLogInit(WIDTH,HEIGHT,ScanLogBuff,&ScanLog);

	//set mouse starting cell
	MapSetWall( 0, 0, EAST, true, &WorldMap );
	MapSetWall( 0, 0, EAST, true, &MouseMap );
	ScanLogSet( 0, 0, true, &ScanLog );
	

	//Populate the world map with sample maze
	MakeLines(0, 2, 6, EAST,  SOUTH, &WorldMap);
	MakeLines(6, 3, 4, NORTH, WEST,  &WorldMap);
	MakeLines(1, 5, 6, EAST,  NORTH, &WorldMap );
	MakeLines(4, 3, 2, NORTH, WEST,  &WorldMap );
	MakeLines(4, 2, 2, EAST,  NORTH, &WorldMap );
	MakeLines(0, 5, 4, EAST,  SOUTH, &WorldMap );
	MapSetWall(1, 6, EAST, true,     &WorldMap );
	MakeLines(3, 3, 2, NORTH, WEST, &WorldMap );
	//Set up flood fill.
	FloodFillInit(
			WIDTH,
			HEIGHT,
			FloodMapBuff,
			FloodEventBuff,
			&FloodMap);

	INDEX x=0, y=0;
	enum DIRECTION dir=NORTH;
	RunMoves( &x,&y,&dir, WIDTH/2, HEIGHT/2 );

	printf("explore done\n");

	RunMoves(&x,&y,&dir,0,0);

	printf("return done\n");
	return 0;
}
