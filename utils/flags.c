#include"flags.h"

void FlagsClear(FLAG_WORD * flags, COUNT numFlags)
{
	INDEX end = FlagSize( numFlags );
	INDEX cur;
	for( cur=0; cur<end; cur++ )
		flags[cur] = 0;
}

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
