#include"bitmap.h"
#include<stdio.h>
#include<stdlib.h>

/*
 * Tests the flag unit. Prints the flag buffer to termainal for validation.
 */

int main()
{
	COUNT numFlags;
	INDEX input;
	COUNT inputNum;
	BITMAP_WORD * flags;

	printf("Enter a number of flags\n");
	scanf("%ld", &numFlags);

	printf("Need %ld bytes\n", BitmapSize( numFlags ) * sizeof( BITMAP_WORD ) );

	flags = malloc( BitmapSize( numFlags ) * sizeof( BITMAP_WORD ) );
	
	do
	{
		BitmapPrint( flags, numFlags );
		printf("1 Set, 2 Clear, 3 Find First Flag, 4 Get Flag, 5 Quit\n");
		scanf("%ld", & input );
		switch( input )
		{
			case 1:
				printf("what index?");
				scanf("%ld",&inputNum);
				BitmapOn( flags, inputNum );
				break;
			case 2:
				printf("what index?");
				scanf("%ld",&inputNum);
				BitmapOff( flags, inputNum );
				break;
			case 3:
				printf("looking for flag\n");
				printf("first flag was %d\n", (int) BitmapGetFirstFlag( flags, numFlags ));
				break;
			case 4:
				printf("what index?");
				scanf("%ld",&inputNum);
				if( BitmapGet( flags, inputNum ) ) {
					printf("SET\n");
				} else {
					printf("CLEAR\n");
				}
				break;
			case 5:
				printf("have a good day\n");
				break;
			default:
				continue;
		}
	}while( input != 5 );
	return 0;
}
