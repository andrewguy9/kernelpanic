#include"flags.h"

/*
 * Presents an abstraction for multi word flags.
 * 
 * FlagWordSize - specifies the size of the word in the unit.
 *
 * FlagSize - returns the number of flag words needed to 
 * 	have 1 bit per flag. Rounds appropriatly.
 *
 * FlagOn - Turns a flag on.
 *
 * FlagOff - Turn off a flag
 *
 * FlagGet - returns a flags status.
 *
 * TODO RENAME THIS UNIT TO BITMAP
 */

#ifdef PC_BUILD
#include<stdio.h>

/*
 * Prints out a flag buffer's status to the terminal.
 */
void FlagsPrint( FLAG_WORD flags[], COUNT numFlags )
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
	printf("|\n");
}
#endif

/*
 * Clears all the flags in a flag buffer.
 */
void FlagsClear(FLAG_WORD * flags, COUNT numFlags)
{
	INDEX end = FlagSize( numFlags );
	INDEX cur;
	for( cur=0; cur<end; cur++ )
		flags[cur] = 0;
}

/*
 * Iterates through a buffer and returns the first true flag.
 */
INDEX FlagsGetFirstFlag(FLAG_WORD * flags, COUNT numFlags)
{
	INDEX end = FlagSize( numFlags );
	INDEX index;
	INDEX offset;
	for( index = 0; index < end; index++ )
	{
		if( flags[index] != 0 )
		{
			//we are in the right byte
			for( offset = 0; offset < FlagWordSize; offset++ )
			{
				if( FlagGet(flags, index*FlagWordSize+offset) )
				{
					return index*FlagWordSize+offset;
				}
			}
			return -2;
		}
	}
	return -1;
}
