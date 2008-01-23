#include"map.h"
#include<stdio.h>

//
//The map only does measurements based on the south west.
//North and east are constructed from sw coordinates.
//
void PrintLine( COUNT numChars )
{
	for(; numChars > 0; numChars--)
	{
		printf("-");
	}
}

void PrintMap( struct MAP * map )
{
	INDEX x, y;
	PrintLine( map->Width*4);

	for( y = map->Height; y>=0; y--)
	{
		//construct cells
		for(x=0; x<map->Width; x++)
		{
			printf("| ");
		}
		printf("\n");
	}
}

int main()
{
	COUNT width=16, height=16;
	struct MAP map;
	FLAG_WORD wallBuff[ MapSizeNeeded(width, height) ];

	MapInit( &map, wallBuff, MapSizeNeeded(width, height), width, height );

	PrintMap( & map );
	return 0;
}
