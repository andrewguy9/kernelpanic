#include"floodfill.h"
#include"map.h"

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

struct MAP Map;
struct FLOOD_MAP FloodMap;

char MapBuff[ MapSizeNeeded(16,16)];
char FloodMapBuff[FLOOD_MAP_SIZE(16,16)];
char FloodEventBuff[FLOOD_EVENT_SIZE(16,16)];

int main()
{
	MapInit( &Map, MapBuff, MapSizeNeeded(16,16), 16, 16 );
	FloodFillInit( 
			16, 
			16, 
			FloodMapBuff, 
			FloodEventBuff, 
			&FloodMap );

	FloodFillClear( &FloodMap );
	FloodFillSetDestination( 8, 8, &FloodMap );
	FloodFillCalculate( &Map, &FloodMap );

	PrintMap( &Map, &FloodMap );

	return 0;
}
