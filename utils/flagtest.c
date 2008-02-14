#include"flags.h"
#include<stdio.h>

void PrintFlags( FLAG_WORD flags[], COUNT numFlags )
{
	INDEX count;
	printf("|");
	for(count = 0; count < numFlags; count++)
	{
		if( FlagGet( flags, count ) )
		{
			printf("+");
		}
		else
		{
			printf("-");
		}
	}
	printf("|");
}

int main()
{
	COUNT numFlags;
	INDEX input;
	COUNT inputNum;
	FLAG_WORD * flags;

	printf("Enter a number of flags\n");
	scanf("%d", &numFlags);

	printf("Need %d bytes\n", FlagSize( numFlags ) * sizeof( FLAG_WORD ) );

	flags = malloc( FlagSize( numFlags ) * sizeof( FLAG_WORD ) );
	
	do
	{
		PrintFlags( flags, numFlags );
		printf("1 Set, 2 Clear, 3 Find First Flag, 4 Quit\n");
		scanf("%d", & input );
		switch( input )
		{
			case 1:
				printf("what index?");
				scanf("%d",&inputNum);
				FlagOn( flags, inputNum );
				break;
			case 2:
				printf("what index?");
				scanf("%d",&inputNum);
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
