/*
===========================================================================

idBitField provides operations on a bit field stored as an array of 32-bit
words.

===========================================================================
*/
#ifndef __BITFIELD_H__
#define __BITFIELD_H__

#include <string.h>

class idBitField {
public:
	static bool Test( const int *words, int bitNum ) {
		assert( words != NULL && bitNum >= 0 );
		uint32 word;
		memcpy( &word, &words[bitNum >> 5], sizeof( word ) );
		return ( word & ( 1u << ( bitNum & 31 ) ) ) != 0;
	}

	static void Set( int *words, int bitNum ) {
		assert( words != NULL && bitNum >= 0 );
		uint32 word;
		memcpy( &word, &words[bitNum >> 5], sizeof( word ) );
		word |= 1u << ( bitNum & 31 );
		memcpy( &words[bitNum >> 5], &word, sizeof( word ) );
	}

	static void Clear( int *words, int bitNum ) {
		assert( words != NULL && bitNum >= 0 );
		uint32 word;
		memcpy( &word, &words[bitNum >> 5], sizeof( word ) );
		word &= ~( 1u << ( bitNum & 31 ) );
		memcpy( &words[bitNum >> 5], &word, sizeof( word ) );
	}
};

#endif
