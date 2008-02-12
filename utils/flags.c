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
	INDEX cur;
	INDEX offset;
	for( cur = 0; cur < end; cur++ )
	{
		if( flags[cur] != 0 )
		{
			for( offset = 0; offset < FlagWordSize; offset ++ )
				if( FlagGenerateMask( offset ) & flags[cur] != 0 )
					return cur * FlagWordSize + offset;
		}
	}
	return -1;
}
