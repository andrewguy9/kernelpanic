#include"move.h"
#include"map.h"
#include"floodfill.h"

#include<stdio.h>
#include<stdlib.h>

#define WIDTH 8
#define HEIGHT 8

void PrintMap( struct MAP * map, struct FLOOD_MAP * flood)
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
				//flood value
				printf("%3x",FloodFillGet( x, y, flood));
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

struct MAP Map;
struct FLOOD_MAP FloodMap;

char MapBuff[ MapSizeNeeded(WIDTH,HEIGHT)];
char FloodMapBuff[FLOOD_MAP_SIZE(WIDTH,HEIGHT)];
char FloodEventBuff[FLOOD_EVENT_SIZE(WIDTH,HEIGHT)];

void PrintMove( struct MOVE * move )
{
	if( move == &MoveNowhere)
		printf("move nowhere ");
	else if (move == &MoveStright)
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

void RunMoves()
{
	INDEX x=0, y=0;
	enum DIRECTION dir = NORTH;
	struct MOVE * move = &MoveNowhere;

	while( FloodFillGet( x, y, &FloodMap) != 0 )
	{
		printf("dist = %d ", FloodFillGet( x, y, &FloodMap));
		PrintState(x,y,dir);

		if( x > WIDTH || y > HEIGHT )
		{
			printf("out of bounds\n");
			exit(0);
		}

		move = MoveFindBest( x, y, dir, &FloodMap, &Map );

		PrintMove( move );

		MoveApply( &x, &y, &dir, move );

		printf("now at ");
		PrintState(x,y,dir);

		printf("dist = %d ", FloodFillGet( x, y, &FloodMap));

		printf("\n");

	}
	printf("Ended up at (%d,%d)\n",x,y);
}

int main()
{
	MapInit( &Map, MapBuff, MapSizeNeeded(WIDTH,HEIGHT), WIDTH, HEIGHT );

	MakeLines( 2, 3, &Map, EAST );
	MakeLines( 2, 2, &Map, SOUTH );

	FloodFillInit( 
			WIDTH, 
			HEIGHT, 
			FloodMapBuff, 
			FloodEventBuff, 
			&FloodMap );

	FloodFillClear( &FloodMap );
	FloodFillSetDestination( WIDTH/2, HEIGHT/2, &FloodMap );
	FloodFillSetDestination( WIDTH/2-1, HEIGHT/2, &FloodMap );
	FloodFillSetDestination( WIDTH/2, HEIGHT/2-1, &FloodMap );
	FloodFillSetDestination( WIDTH/2-1, HEIGHT/2-1, &FloodMap );

	FlagsPrint( FloodEventBuff, WIDTH*HEIGHT );

	FloodFillCalculate( &Map, &FloodMap );

	PrintMap( &Map, &FloodMap );

	RunMoves( );
	
	printf("done\n");
	return 0;
}
