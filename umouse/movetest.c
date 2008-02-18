#include"move.h"
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

void MakeLines(COUNT hStart, COUNT vStart, COUNT hEnd, COUNT vEnd, COUNT hSkip, COUNT vSkip, struct MAP * map, enum DIRECTION dir )
{
	INDEX x,y;
	for(x=hStart; x<hEnd; x+=hSkip)
		for(y=vStart; y<vEnd; y+=vSkip)
			MapSetWall( x, y, dir, TRUE, map );
}

struct MAP MouseMap;
struct MAP WorldMap;
struct FLOOD_MAP FloodMap;
struct SCAN_LOG ScanLog;

char MouseMapBuff[ MapSizeNeeded(WIDTH,HEIGHT) ];
char WorldMapBuff[ MapSizeNeeded(WIDTH,HEIGHT) ];
char FloodMapBuff[ FLOOD_MAP_SIZE(WIDTH,HEIGHT) ];
char FloodEventBuff[ FLOOD_EVENT_SIZE(WIDTH,HEIGHT) ];
char ScanLogBuff[ SCAN_LOG_SIZE(WIDTH,HEIGHT) ];

void PrintMove( struct MOVE * move )
{
	if( move == &MoveNowhere)
		printf("move nowhere ");
	else if (move == &MoveStraight)
		printf("move straight ");
	else if (move == &MoveBack)
		printf("move back ");
	else if (move == &MoveLeft)
		printf("move left ");
	else if (move == &MoveRight)
		printf("move right ");
	else if (move == &MoveIntegratedLeft)
		printf("move integrated left ");
	else if( move == &MoveIntegratedRight)
		printf("move integrated right ");
	else if( move == &MoveTurnLeft)
		printf("move turning left ");
	else if( move == &MoveTurnRight)
		printf("move turning right ");
	else if ( move == &MoveTurnBack )
		printf("move turning back " );
	else if ( move == &MoveHairpinLeft )
		printf("move hairpin left ");
	else if ( move == &MoveHairpinRight )
		printf("move hairpin right ");
	else
	{
		printf("NOT POSSIBLE MOVE\n");
		exit(0);
	}
}

void PrintState(INDEX x, INDEX y, enum DIRECTION dir )
{
	printf("(%d,%d) ",x,y);
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
}

void UpdateMap( INDEX x, INDEX y, enum DIRECTION dir, INDEX destX, INDEX destY)
{
	INDEX scanX, scanY;
	BOOL mapChanged = FALSE;
	enum DIRECTION testDir;

	scanX = x;
	scanY = y;

	//calc coordinates of cell in front of mouse.
	MoveApply( &scanX, &scanY, &dir, &MoveStraight );

	//see if there is a wall infront of us
	if( ! MapGetWall( x, y, dir, &WorldMap ))
	{
		printf("scanning\n");
		//no wall so we can scan.
		//check walls for changes to update floodfill
		for(testDir = NORTH; testDir <= WEST; testDir++)
		{
			if( ! MapGetWall( scanX, scanY, testDir, &MouseMap ) &&
					MapGetWall( scanX, scanY, testDir, &WorldMap ) )
			{//mouse just saw new wall
				MapSetWall( scanX, scanY, testDir, TRUE, &MouseMap );
				mapChanged = TRUE;
			}
		}
		//Log that we scanned the cell.
		ScanLogSet(scanX,scanY,TRUE,&ScanLog);
	}
	else
	{
		printf("cant scan\n");
	}

	if( mapChanged )
	{
		printf( "found new walls at (%d,%d)\n", scanX, scanY );
		FloodFillClear( &FloodMap );
		FloodFillSetDestination( destX, destY, &FloodMap );
		FloodFillCalculate( &MouseMap, &FloodMap );
	}

	printf("world map\n");
	PrintMapScan( &WorldMap, &ScanLog, x, y, dir );
	printf("mouse map\n");
	PrintMapFlood( &MouseMap, &FloodMap, x, y, dir );
}

void RunMoves(INDEX * startX, INDEX *startY, INDEX *startDir, INDEX destX, INDEX destY )
{
	//Mouse state variables.
	INDEX x= *startX, y= *startY;
	enum DIRECTION dir = *startDir;
	struct MOVE * move = &MoveNowhere;
	COUNT straightAway; 

	//force a flood fill calculation
	FloodFillClear( &FloodMap );
	FloodFillSetDestination( destX, destY, &FloodMap );
	FloodFillCalculate( &MouseMap, &FloodMap );

	//start makeing moves
	do
	{
		printf("------------------------------------------------------\n");

		//see if we are in straight away.
		straightAway = MoveStraightAwayLength(
				x,
				y,
				dir,
				&MouseMap,
				&FloodMap,
				&ScanLog);
		if( straightAway > 0 )
		{
			printf("speeding %d cells in straight away\n", straightAway);
			while(straightAway>0)
			{
				MoveApply( &x, &y, &dir, &MoveStraight );
				straightAway--;
			}
		}

		//Map the area infront of you
		UpdateMap( x, y, dir, destX, destY );

		//Print the state
		printf("dist = %d ", FloodFillGet( x, y, &FloodMap));
		PrintState( x, y, dir );
		if( x > WIDTH || y > HEIGHT )
		{
			printf("out of bounds\n");
			exit(0);
		}

		//Make the decision
		move = MoveFindBest( x, y, dir, &FloodMap, &MouseMap, &ScanLog );
		PrintMove( move );

		//Make the move
		MoveApply( &x, &y, &dir, move );

		printf("now at ");
		PrintState(x,y,dir);

		printf("dist = %d \n", FloodFillGet( x, y, &FloodMap));

	}while( FloodFillGet( x, y, &FloodMap) != 0 );

	printf("Ended up at (%d,%d)\n",x,y);
	printf("world map\n");
	PrintMapFlood( &WorldMap, &FloodMap, x, y, dir );
	printf("mouse map\n");
	PrintMapFlood( &MouseMap, &FloodMap, x, y, dir );

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
	MapSetWall( 0, 0, EAST, TRUE, &WorldMap );
	MapSetWall( 0, 0, EAST, TRUE, &MouseMap );
	ScanLogSet( 0, 0, TRUE, &ScanLog );

	//Populate the world map with sample maze
	MakeLines( 0, 0, WIDTH, HEIGHT, 2, 3, &WorldMap, EAST );
	MakeLines( 0, 0, WIDTH, HEIGHT, 2, 2, &WorldMap, SOUTH );
	MakeLines( 0, 1, WIDTH/2, HEIGHT, 1, 8, &WorldMap, NORTH );	
	MakeLines( 4, 4, WIDTH, HEIGHT, 8, 1, &WorldMap, EAST );

	//Set up flood fill.
	FloodFillInit( 
			WIDTH, 
			HEIGHT, 
			FloodMapBuff, 
			FloodEventBuff, 
			&FloodMap );

	INDEX x=0,y=0;
	enum DIRECTION dir=NORTH;
	RunMoves( &x,&y,&dir, WIDTH/2, HEIGHT/2 );

	printf("explore done\n");

	RunMoves(&x,&y,&dir,0,0);

	printf("return done\n");
	return 0;
}
