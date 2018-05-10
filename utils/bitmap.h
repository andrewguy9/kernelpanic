#ifndef BITMAP_H
#define BITMAP_H

#include"utils.h"

/*
 * TODO Bitmap library should be revamped to look more like buffer.
 * If you are passing bitmaps around, there should be a size field which
 * could be used for ensuring range checks.
 * There could be definition macros so that size is well defined.
 */
//
//  Private Macros for doing library math
//
typedef int BITMAP_WORD;
#define BitmapWordSize (sizeof(BITMAP_WORD) * 8)

#define BitmapIndex( Index ) ((Index)/(BitmapWordSize))
#define BitmapOffset( Index ) ((Index)%((BitmapWordSize)))

#define BitmapGenerateMask( Offset ) ( 1 << (Offset) )


//
// Public Macros for doing library math.
//

//Get the size needed for a buffer.
#define BitmapSize( NumElements ) \
	( (NumElements)/(BitmapWordSize) + ((NumElements)%(BitmapWordSize) ? 1 : 0 ) )

//Test if a flag is on
#define BitmapGet( Map, Index ) (((Map)[BitmapIndex(Index)] & BitmapGenerateMask( BitmapOffset( Index ))) != 0)

//Turn a flag on
#define BitmapOn( Map, Index ) ((Map)[BitmapIndex(Index)] |= BitmapGenerateMask( BitmapOffset( Index ) ) )  

//Turn a flag off
#define BitmapOff( Map, Index ) ((Map)[BitmapIndex(Index)] &= ~ BitmapGenerateMask( BitmapOffset( Index ) ) )

//Toggle a flag
#define BitmapToggle( Map, Index ) ((Map)[BitmapIndex(Index)] ^= BitmapGenerateMask( BitmapOffset(Index) ) )

//Public functions to do flag work
#ifdef PC_BUILD
void BitmapPrint( BITMAP_WORD map[], COUNT numBits );
#endif

void BitmapClear(BITMAP_WORD * map, COUNT numBits);

INDEX BitmapGetFirstFlag(BITMAP_WORD * map, COUNT numBits);

//TODO It would be nice to have a BitmapGetBlock 
//function to work in allocators and bloom filters.

#endif
