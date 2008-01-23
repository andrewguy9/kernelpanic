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
	char input;
	COUNT inputNum;
	FLAG_WORD * flags;

	printf("Enter a number of flags\n");
	scanf("%d", &numFlags);

	flags = malloc( FlagSize( numFlags ) * sizeof( FLAG_WORD ) );
	
	do
	{
		PrintFlags( flags, numFlags );
		printf("Set, Clear, Quit\n");
		scanf("%c\n", & input );
		switch( input )
		{
			case 's':
				printf("what index?");
				scanf("%d",&inputNum);
				FlagOn( flags, inputNum );
				break;
			case 'c':
				printf("what index?");
				scanf("%d",&inputNum);
				FlagOff( flags, inputNum );
				break;
			case 'q':
				printf("have a good day");
				break;
			default:
				continue;
		}
	}while( input != 'q' );
}
