#include"bitmap.h"

/*
 * Presents an abstraction for multi word bitmaps.
 * 
 * BitmapWordSize - specifies the size of the word in the unit.
 *
 * BitmapSize - returns the number of BITMAP_WORDs needed for
 * some number of bits. Rounds appropriatly.
 *
 * BitmapOn - Turns a bit on.
 *
 * BitmapOff - Turn off a bit
 *
 * BitmapGet - returns a bit's status.
 */

#ifdef PC_BUILD
#include<stdio.h>

/*
 * Prints out a bitmap to the terminal.
 */
void BitmapPrint( BITMAP_WORD map[], COUNT numBits )
{
	INDEX count;
	printf("|");
	for(count = 0; count < numBits; count++)
	{
		if( BitmapGet( map, count ) )
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
 * Clears all the bits in a bitmap.
 */
void BitmapClear(BITMAP_WORD * map, COUNT numBits)
{
	INDEX end = BitmapSize( numBits );
	INDEX cur;
	for( cur=0; cur<end; cur++ )
		map[cur] = 0;
}

/*
 * Iterates through a buffer and returns the first true bit.
 */
INDEX BitmapGetFirstFlag(BITMAP_WORD * map, COUNT numBits)
{
	INDEX end = BitmapSize( numBits );
	INDEX index;
	INDEX offset;
	for( index = 0; index < end; index++ )
	{
		if( map[index] != 0 )
		{
			//we are in the right byte
			for( offset = 0; offset < BitmapWordSize; offset++ )
			{
				if( BitmapGet(map, index*BitmapWordSize+offset) )
				{
					return index*BitmapWordSize+offset;
				}
			}
			return -2;
		}
	}
	return -1;
}
