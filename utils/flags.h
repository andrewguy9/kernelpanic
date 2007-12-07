#ifndef FLAGS_H
#define FLAGS_H

#include"utils.h"

//
//  Private Macros for doing library math
//
#define FlagWordSize sizeof(char)
#define FlagSize( NumElements ) ( (NumElements)/(FlagWordSize) + (((NumElements)%(FlagWordSize)) ? 1: 0 ) )

#define FlagIndex( Index ) ((Index)/(FlagWordSize*8))
#define FlagOffset( Index ) ((Index%(FlagWordSize*8)))

#define FlagGenerateMask( Offset ) ( 1 << (Offset) )


//
// Public Macros for doing library math.
//

//Test if a flag is on
#define FlagGet( Flags, Index ) (Flags[FlagIndex(Index)] & FlagGenerateMask( FlagOffset( Index ) ) )

//Turn a flag on
#define FlagOn( Flags, Index ) (Flags[FlagIndex(Index)] |= FlagGenerateMask( FlagOffset( Index ) ) )  

//Turn a flag off
#define FlagOff( Flags, Index ) (Flags[FlagIndex(Index)] &= ~ FlagGenerateMask( FlagOffset( Index ) ) )

#endif
