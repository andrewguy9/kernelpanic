#include"move.h"
#include"map.h"
#include"floodfill.h"

#include<stdio.h>
#include<stdlib.h>

#define WIDTH 8
#define HEIGHT 8

void PrintMap( struct MAP * map, struct FLOOD_MAP * flood, INDEX mouseX, INDEX mouseY, enum DIRECTION mouseDir)
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

void MakeLines(COUNT hSkip, COUNT vSkip, struct MAP * map, enum DIRECTION dir )
{
	INDEX x,y;
	for(x=0; x<map->Width; x+=hSkip)
		for(y=0; y<map->Height; y+=vSkip)
			MapSetWall( x, y, dir, TRUE, map );
}

struct MAP MouseMap;
struct MAP WorldMap;
struct FLOOD_MAP FloodMap;

char MouseMapBuff[ MapSizeNeeded(WIDTH,HEIGHT) ];
char WorldMapBuff[ MapSizeNeeded(WIDTH,HEIGHT) ];
char FloodMapBuff[ FLOOD_MAP_SIZE(WIDTH,HEIGHT) ];
char FloodEventBuff[ FLOOD_EVENT_SIZE(WIDTH,HEIGHT) ];

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
		printf("move interated left ");
	else if( move == &MoveIntegratedRight)
		printf("move integrated right ");
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

void UpdateMap( INDEX x, INDEX y, enum DIRECTION dir )
{
	INDEX newX, newY;
	BOOL mapChanged = FALSE;
	enum DIRECTION testDir;

	newX = x;
	newY = y;

	MoveApply( &newX, &newY, &dir, &MoveStraight );

	//now xy is sensor position.
	//check walls for changes to update floodfill
	for(testDir = NORTH; testDir <= WEST; testDir++)
	{
		if( ! MapGetWall( newX, newY, testDir, &MouseMap ) &&
				MapGetWall( newX, newY, testDir, &WorldMap ) )
		{//mouse just saw new wall
			MapSetWall( newX, newY, testDir, TRUE, &MouseMap );
			mapChanged = TRUE;
		}
	}

	if( mapChanged )
	{
		printf( "found new walls at (%d,%d)\n", newX, newY );
		FloodFillClear( &FloodMap );
		FloodFillSetDestination( WIDTH/2, HEIGHT/2, &FloodMap );
		FloodFillCalculate( &MouseMap, &FloodMap );
	}

	printf("world map\n");
	PrintMap( &WorldMap, &FloodMap, x, y, dir );
	printf("mouse map\n");
	PrintMap( &MouseMap, &FloodMap, x, y, dir );
}

void RunMoves()
{
	//Mouse state variables.
	INDEX x=0, y=0;
	enum DIRECTION dir = NORTH;
	struct MOVE * move = &MoveNowhere;

	do
	{
		//Map the area infront of you
		UpdateMap( x, y, dir );

		//Print the state
		printf("dist = %d ", FloodFillGet( x, y, &FloodMap));
		PrintState( x, y, dir );
		if( x > WIDTH || y > HEIGHT )
		{
			printf("out of bounds\n");
			exit(0);
		}

		//Make the decision
		move = MoveFindBest( x, y, dir, &FloodMap, &MouseMap );
		PrintMove( move );

		//Make the move
		MoveApply( &x, &y, &dir, move );

		printf("now at ");
		PrintState(x,y,dir);

		printf("dist = %d ", FloodFillGet( x, y, &FloodMap));

		printf("\n");

	}while( FloodFillGet( x, y, &FloodMap) != 0 );
	printf("Ended up at (%d,%d)\n",x,y);
}

int main()
{
	//Initialize Maps
	MapInit( &WorldMap, WorldMapBuff, MapSizeNeeded(WIDTH,HEIGHT), WIDTH, HEIGHT );
	MapInit( &MouseMap, MouseMapBuff, MapSizeNeeded(WIDTH,HEIGHT), WIDTH, HEIGHT );
	MapSetWall( 0, 0, EAST, TRUE, &WorldMap );
	MapSetWall( 0, 0, EAST, TRUE, &MouseMap );

	//Populate the world map with sample maze
	MakeLines( 2, 3, &WorldMap, EAST );
	MakeLines( 2, 2, &WorldMap, SOUTH );

	//Set up flood fill.
	FloodFillInit( 
			WIDTH, 
			HEIGHT, 
			FloodMapBuff, 
			FloodEventBuff, 
			&FloodMap );

	//Start the mouse
	RunMoves( );
	
	printf("done\n");
	printf("final state\n");

	printf("world map\n");
	PrintMap( &WorldMap, &FloodMap, 0, 0, NORTH );
	printf("mouse map\n");
	PrintMap( &MouseMap, &FloodMap, 0, 0, NORTH );
	
	return 0;
}
