#include"map.h"
#include<stdio.h>

/*
 * Tests the map unit. Prints the map to terminal for validation.
 */

struct MAP MyMap;
BITMAP_WORD MyWallBuff[ MapSizeNeeded(MAP_WIDTH, MAP_HEIGHT) ];


//
//The map only does measurements based on the south west.
//North and east are constructed from sw coordinates.
//
void PrintMap( struct MAP * map )
{
	INDEX x, y;

	for(y = map->Height; y != -1; y--) {

		//Construct west Wall and cell label
		for(x = 0; x < map->Width; x++) {
			if( MapGetWall(x, y, WEST, map) ) {
				printf("|");
			} else {
				printf(" ");
			}
			printf("%1lx,%1lx", x, y);
		}
		//Construct the final west wall
		if( MapGetWall(map->Width, y, WEST, map) ) {
			printf("|\n");
		} else {
			printf(" \n");
		}

		//Construct South Wall
		for(x = 0; x < map->Width; x++) {
			printf("+");
			if( MapGetWall(x, y, SOUTH, map) ) {
				printf("---");
			} else {
				printf("   ");
			}
		}
		//Construct the final peg in the row.
		printf("+\n");


	}
}

void PrintFlags( struct MAP * map )
{
	printf("Vertical Walls:  ");
	BitmapPrint(map->VWalls, MapBitsNeeded(map->Height, map->Height));
	printf("Horizontal Walls:");
	BitmapPrint(map->HWalls, MapBitsNeeded(map->Width, map->Height));
}

void PrintWalls( struct MAP * map, INDEX x, INDEX y )
{
	printf("%lx,%lx : ",x,y);
	if( MapGetWall(x,y,NORTH, map ) )
		printf("N");
	else
		printf(" ");
	if( MapGetWall(x,y,SOUTH, map ) )
		printf("S");
	else
		printf(" ");
	if( MapGetWall(x,y,EAST, map ) )
		printf("E");
	else
		printf(" ");
	if( MapGetWall(x,y,WEST, map ) )
		printf("W");
	else
		printf(" ");
	printf("\n");
}

int main()
{
	enum DIRECTION dir;
	INDEX x,y,dir_int;

	MapInit( &MyMap, MyWallBuff, MapSizeNeeded(MAP_WIDTH, MAP_HEIGHT), MAP_WIDTH, MAP_HEIGHT );

    PrintMap( & MyMap );

	PrintFlags( & MyMap );

	while( TRUE )
	{
		printf("please enter x then y then direction (1n 2s 3e 4w)\n");
		scanf("%ld %ld %ld", &x, &y, &dir_int);

		switch(dir_int)
		{
			case 1:
				dir = NORTH;
				break;
			case 2:
				dir = SOUTH;
				break;
			case 3:
				dir = EAST;
				break;
			case 4:
				dir = WEST;
				break;
		}

		MapSetWall( x, y, dir, TRUE, &MyMap );

		PrintMap( & MyMap );
		PrintFlags( & MyMap );
		PrintWalls( & MyMap, x, y );
	}

	return 0;
}
