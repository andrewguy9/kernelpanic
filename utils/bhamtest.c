#include"bham.h"
#include"utils.h"
#include<stdio.h>

/*
 * Tests the bham unit.
 * Prints the slope to the screen for validation.
 */

struct BHAM Bham;

void PrintGraph( struct BHAM * bham, COUNT maxTicks )
{
	COUNT ticks;
	COUNT spaces = 0, curSpaces;
	for( ticks = 0; ticks < maxTicks; ticks++ )
	{
		//Update ticks
		if( BHamTick( bham ) )
		{
			spaces++;
			printf("+");
		}
		else
		{
			printf("-");
		}

		//draw line.
		for( curSpaces =0; curSpaces < spaces; curSpaces++ )
		{
			printf(" ");
		}
		printf("*\n");
	}
}

int main()
{
	COUNT x,y,ticks;
	printf( "Enter x y ticks\n" );
	scanf( "%d %d %d", &x,&y,&ticks );
	BHamInit( x,y,&Bham );

	PrintGraph( &Bham, ticks );
	return 0;
}
