#include"floodfill.h"
#include"map.h"
#include"utils/bitmap.h"
#define WIDTH 8
#define HEIGHT 8

/*
 * Tests the floodfloodfill unit.
 * Prints out the flood map to the terminal for verification.
 */

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
			MapSetWall( x, y, dir, true, map );
}

struct MAP Map;
struct FLOOD_MAP FloodMap;

BITMAP_WORD MapBuff[ MapSizeNeeded(WIDTH,HEIGHT)];
BITMAP_WORD FloodMapBuff[FLOOD_MAP_SIZE(WIDTH,HEIGHT)];
BITMAP_WORD FloodEventBuff[FLOOD_EVENT_SIZE(WIDTH,HEIGHT)];

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

	BitmapPrint( FloodEventBuff, WIDTH*HEIGHT );

	FloodFillCalculate( &Map, &FloodMap );

	PrintMap( &Map, &FloodMap );

	return 0;
}
