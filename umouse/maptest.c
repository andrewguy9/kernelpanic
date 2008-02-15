#include"map.h"
#include<stdio.h>

/*
 * Tests the map unit. Prints the map to terminal for validation.
 */

//
//The map only does measurements based on the south west.
//North and east are constructed from sw coordinates.
//
void PrintMap( struct MAP * map )
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
				printf("%1x,%1x",x,y);
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

void PrintBytes( struct MAP * map )
{
	INDEX index;

	printf("Vertical Walls:  ");
	for( index = 0; index < MapDimensionSizeNeeded( map->Height, map->Width ); index++ )
		printf("%x", map->VWalls[index]);
	printf("\n");

	printf("Horizontal Walls:");
	for( index = 0; index < MapDimensionSizeNeeded( map->Height, map->Width ); index++ )
		printf("%x", map->HWalls[index]);
	printf("\n");
}

void PrintWalls( struct MAP * map )
{
	INDEX x,y;
	for(x = 0; x < 16; x++ )
	{
		for( y=0; y < 16; y++ )
		{
			printf("%x,%x : ",x,y);
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
	}
}

int main()
{
	COUNT width=16, height=16;
	enum DIRECTION dir;
	INDEX x,y,dir_int;
	struct MAP map;

	FLAG_WORD wallBuff[ MapSizeNeeded(width, height) ];

	MapInit( &map, wallBuff, MapSizeNeeded(width, height), width, height );

	PrintMap( & map );

	//PrintBytes( & map );

	//PrintWalls( & map );
	
	while( TRUE )
	{
		printf("please enter x then y then direction (1n 2s 3e 4w)\n");
		scanf("%d %d %d", &x, &y, &dir_int);

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

		MapSetWall( x, y, dir, TRUE, &map );

		PrintMap( &map );
	}

	return 0;
}
