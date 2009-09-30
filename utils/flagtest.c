#include"flags.h"
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
	FLAG_WORD * flags;

	printf("Enter a number of flags\n");
	scanf("%ld", &numFlags);

	printf("Need %ld bytes\n", FlagSize( numFlags ) * sizeof( FLAG_WORD ) );

	flags = malloc( FlagSize( numFlags ) * sizeof( FLAG_WORD ) );
	
	do
	{
		FlagsPrint( flags, numFlags );
		printf("1 Set, 2 Clear, 3 Find First Flag, 4 Quit\n");
		scanf("%ld", & input );
		switch( input )
		{
			case 1:
				printf("what index?");
				scanf("%ld",&inputNum);
				FlagOn( flags, inputNum );
				break;
			case 2:
				printf("what index?");
				scanf("%ld",&inputNum);
				FlagOff( flags, inputNum );
				break;
			case 3:
				printf("looking for flag\n");
				printf("first flag was %d\n", (int) FlagsGetFirstFlag( flags, numFlags ));
				break;
			case 4:
				printf("have a good day\n");
				break;
			default:
				continue;
		}
	}while( input != 4 );
	return 0;
}
