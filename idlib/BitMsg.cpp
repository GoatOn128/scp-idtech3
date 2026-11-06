/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").  

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#pragma hdrstop
#include "precompiled.h"
#include "Huffman.h"

static idHuffman bitMsgHuffman;

static void InitBitMsgHuffman() {
	static bool initialized = false;
	if ( initialized ) return;
	initialized = true;
	static const int frequencies[256] = { 250315, 41193, 6292, 7106, 3730, 3750, 6110, 23283, 33317, 6950, 7838, 9714, 9257, 17259, 3949, 1778, 8288, 1604, 1590, 1663, 1100, 1213, 1238, 1134, 1749, 1059, 1246, 1149, 1273, 4486, 2805, 3472, 21819, 1159, 1670, 1066, 1043, 1012, 1053, 1070, 1726, 888, 1180, 850, 960, 780, 1752, 3296, 10630, 4514, 5881, 2685, 4650, 3837, 2093, 1867, 2584, 1949, 1972, 940, 1134, 1788, 1670, 1206, 5719, 6128, 7222, 6654, 3710, 3795, 1492, 1524, 2215, 1140, 1355, 971, 2180, 1248, 1328, 1195, 1770, 1078, 1264, 1266, 1168, 965, 1155, 1186, 1347, 1228, 1529, 1600, 2617, 2048, 2546, 3275, 2410, 3585, 2504, 2800, 2675, 6146, 3663, 2840, 14253, 3164, 2221, 1687, 3208, 2739, 3512, 4796, 4091, 3515, 5288, 4016, 7937, 6031, 5360, 3924, 4892, 3743, 4566, 4807, 5852, 6400, 6225, 8291, 23243, 7838, 7073, 8935, 5437, 4483, 3641, 5256, 5312, 5328, 5370, 3492, 2458, 1694, 1821, 2121, 1916, 1149, 1516, 1367, 1236, 1029, 1258, 1104, 1245, 1006, 1149, 1025, 1241, 952, 1287, 997, 1713, 1009, 1187, 879, 1099, 929, 1078, 951, 1656, 930, 1153, 1030, 1262, 1062, 1214, 1060, 1621, 930, 1106, 912, 1034, 892, 1158, 990, 1175, 850, 1121, 903, 1087, 920, 1144, 1056, 3462, 2240, 4397, 12136, 7758, 1345, 1307, 3278, 1950, 886, 1023, 1112, 1077, 1042, 1061, 1071, 1484, 1001, 1096, 915, 1052, 995, 1070, 876, 1111, 851, 1059, 805, 1112, 923, 1103, 817, 1899, 1872, 976, 841, 1127, 956, 1159, 950, 7791, 954, 1289, 933, 1127, 3207, 1020, 927, 1355, 768, 1040, 745, 952, 805, 1073, 740, 1013, 805, 1008, 796, 996, 1057, 11457, 13504 };
	bitMsgHuffman.Init();
	for ( int i = 0; i < 256; ++i ) for ( int j = 0; j < frequencies[i]; ++j ) {
		bitMsgHuffman.AddRef( static_cast<byte>( i ) );
		bitMsgHuffman.DecodeAddRef( static_cast<byte>( i ) );
	}
}

/*
================================================================================================

	idBitMsg

================================================================================================
*/

/*
========================
idBitMsg::CheckOverflow
========================
*/
bool idBitMsg::CheckOverflow( int numBits ) {
	if ( numBits > GetRemainingWriteBits() ) {
		if ( !allowOverflow ) {
			idLib::FatalError( "idBitMsg: overflow without allowOverflow set; maxsize=%i size=%i numBits=%i numRemainingWriteBits=%i",
				GetMaxSize(), GetSize(), numBits, GetRemainingWriteBits() );
		}
		if ( numBits > ( maxSize << 3 ) ) {
			idLib::FatalError( "idBitMsg: %i bits is > full message size", numBits );
		}
		idLib::Printf( "idBitMsg: overflow\n" );
		BeginWriting();
		overflowed = true;
		return true;
	}
	return false;
}

/*
========================
idBitMsg::GetByteSpace
========================
*/
byte *idBitMsg::GetByteSpace( int length ) {
	byte *ptr;

	if ( !writeData ) {
		idLib::FatalError( "idBitMsg::GetByteSpace: cannot write to message" );
	}

	// round up to the next byte
	WriteByteAlign();

	// check for overflow
	CheckOverflow( length << 3 );

	ptr = writeData + curSize;
	curSize += length;
	return ptr;
}

#define NBM( x ) (uint64)( ( 1LL << x ) - 1 )
static uint64 maskForNumBits64[33] = {	NBM( 0x00 ), NBM( 0x01 ), NBM( 0x02 ), NBM( 0x03 ),
										NBM( 0x04 ), NBM( 0x05 ), NBM( 0x06 ), NBM( 0x07 ),
										NBM( 0x08 ), NBM( 0x09 ), NBM( 0x0A ), NBM( 0x0B ),
										NBM( 0x0C ), NBM( 0x0D ), NBM( 0x0E ), NBM( 0x0F ),
										NBM( 0x10 ), NBM( 0x11 ), NBM( 0x12 ), NBM( 0x13 ),
										NBM( 0x14 ), NBM( 0x15 ), NBM( 0x16 ), NBM( 0x17 ),
										NBM( 0x18 ), NBM( 0x19 ), NBM( 0x1A ), NBM( 0x1B ),
										NBM( 0x1C ), NBM( 0x1D ), NBM( 0x1E ), NBM( 0x1F ), 0xFFFFFFFF };

/*
========================
idBitMsg::WriteBits

If the number of bits is negative a sign is included.
========================
*/
void idBitMsg::WriteBits( int value, int numBits ) {
	InitBitMsgHuffman();
	if ( !writeData ) {
		idLib::FatalError( "idBitMsg::WriteBits: cannot write to message" );
	}

	// check if the number of bits is valid
	if ( numBits == 0 || numBits < -31 || numBits > 32 ) {
		idLib::FatalError( "idBitMsg::WriteBits: bad numBits %i", numBits );
	}

	// check for value overflows
	if ( numBits != 32 ) {
		if ( numBits > 0 ) {
			if ( value > ( 1 << numBits ) - 1 ) {
				idLib::FatalError( "idBitMsg::WriteBits: value overflow %d %d", 
									  value, numBits );

			} else if ( value < 0 ) {
				idLib::FatalError( "idBitMsg::WriteBits: value overflow %d %d", 
									  value, numBits );
			}
		} else {
			const unsigned shift = ( -1 - numBits );
			int r = 1 << shift;
			if ( value > r - 1 ) {
				idLib::FatalError( "idBitMsg::WriteBits: value overflow %d %d", 
									  value, numBits );

			} else if ( value < -r ) {
				idLib::FatalError( "idBitMsg::WriteBits: value overflow %d %d", 
									  value, numBits );
			}
		}
	}

	if ( numBits < 0 ) {
		numBits = -numBits;
	}

	if ( oob ) {
		WriteByteAlign();
		if ( numBits == 8 ) {
			writeData[curSize++] = static_cast<byte>( value );
		} else if ( numBits == 16 ) {
			const uint16 v = LittleShort( static_cast<uint16>( value ) );
			memcpy( writeData + curSize, &v, sizeof( v ) );
			curSize += sizeof( v );
		} else if ( numBits == 32 ) {
			const uint32 v = LittleLong( static_cast<uint32>( value ) );
			memcpy( writeData + curSize, &v, sizeof( v ) );
			curSize += sizeof( v );
		} else {
			idLib::FatalError( "idBitMsg: OOB write of %d bits", numBits );
		}
		return;
	}

	// check for msg overflow
	if ( CheckOverflow( numBits ) ) {
		return;
	}

	value &= maskForNumBits64[numBits];
	const int literalBits = numBits & 7;
	for ( int i = 0; i < literalBits; ++i ) {
		idHuffman::PutBit( value & 1, writeData, &huffWriteBit );
		value >>= 1;
	}
	for ( int i = literalBits; i < numBits; i += 8 ) {
		bitMsgHuffman.OffsetTransmit( value & 255, writeData, &huffWriteBit );
		value >>= 8;
	}
	curSize = ( huffWriteBit >> 3 ) + 1;
}

/*
========================
idBitMsg::WriteString
========================
*/
void idBitMsg::WriteString( const char * s, int maxLength, bool make7Bit ) {
	if ( !s ) {
		WriteByte( 0 );
	} else {
		int l = idStr::Length( s );
		if ( maxLength >= 0 && l >= maxLength ) {
			l = maxLength - 1;
		}
		for ( int i = 0; i < l; i++ ) {
			const byte c = static_cast<byte>( s[i] );
			WriteByte( make7Bit && c > 127 ? '.' : c );
		}
		WriteByte( 0 );
	}
}

/*
========================
idBitMsg::WriteData
========================
*/
void idBitMsg::WriteData( const void *data, int length ) {
	if ( oob ) {
		memcpy( GetByteSpace( length ), data, length );
		return;
	}
	const byte *bytes = static_cast<const byte *>( data );
	for ( int i = 0; i < length; ++i ) {
		WriteByte( bytes[i] );
	}
}

/*
========================
idBitMsg::WriteDeltaDict
========================
*/
bool idBitMsg::WriteDeltaDict( const idDict &dict, const idDict *base ) {
	int i;
	const idKeyValue *kv, *basekv;
	bool changed = false;

	if ( base != NULL ) {

		for ( i = 0; i < dict.GetNumKeyVals(); i++ ) {
			kv = dict.GetKeyVal( i );
			basekv = base->FindKey( kv->GetKey() );
			if ( basekv == NULL || basekv->GetValue().Icmp( kv->GetValue() ) != 0 ) {
				WriteString( kv->GetKey() );
				WriteString( kv->GetValue() );
				changed = true;
			}
		}

		WriteString( "" );

		for ( i = 0; i < base->GetNumKeyVals(); i++ ) {
			basekv = base->GetKeyVal( i );
			kv = dict.FindKey( basekv->GetKey() );
			if ( kv == NULL ) {
				WriteString( basekv->GetKey() );
				changed = true;
			}
		}

		WriteString( "" );

	} else {

		for ( i = 0; i < dict.GetNumKeyVals(); i++ ) {
			kv = dict.GetKeyVal( i );
			WriteString( kv->GetKey() );
			WriteString( kv->GetValue() );
			changed = true;
		}
		WriteString( "" );

		WriteString( "" );

	}

	return changed;
}

/*
========================
idBitMsg::ReadBits

If the number of bits is negative a sign is included.
========================
*/
int idBitMsg::ReadBits( int numBits ) const {
	InitBitMsgHuffman();
	int		value;
	bool	sgn;

	if ( !readData ) {
		idLib::FatalError( "idBitMsg::ReadBits: cannot read from message" );
	}

	// check if the number of bits is valid
	if ( numBits == 0 || numBits < -31 || numBits > 32 ) {
		idLib::FatalError( "idBitMsg::ReadBits: bad numBits %i", numBits );
	}

	value = 0;

	if ( numBits < 0 ) {
		numBits = -numBits;
		sgn = true;
	} else {
		sgn = false;
	}

	if ( oob ) {
		ReadByteAlign();
		if ( numBits == 8 ) {
			value = readData[readCount++];
		} else if ( numBits == 16 ) {
			uint16 v;
			memcpy( &v, readData + readCount, sizeof( v ) );
			readCount += sizeof( v );
			value = LittleShort( v );
		} else if ( numBits == 32 ) {
			uint32 v;
			memcpy( &v, readData + readCount, sizeof( v ) );
			readCount += sizeof( v );
			value = LittleLong( v );
		} else {
			idLib::FatalError( "idBitMsg: OOB read of %d bits", numBits );
		}
	} else {
		if ( numBits > ( ( curSize << 3 ) - huffReadBit ) ) {
			return -1;
		}
		const int literalBits = numBits & 7;
		for ( int i = 0; i < literalBits; ++i ) {
			value |= idHuffman::GetBit( const_cast<byte *>( readData ), &huffReadBit ) << i;
		}
		for ( int i = literalBits; i < numBits; i += 8 ) {
			int decoded;
			bitMsgHuffman.OffsetReceive( &decoded, const_cast<byte *>( readData ), &huffReadBit );
			value |= decoded << i;
		}
		readCount = ( huffReadBit >> 3 ) + 1;
	}

	if ( sgn ) {
		if ( value & ( 1 << ( numBits - 1 ) ) ) {
			value |= -1 ^ ( ( 1 << numBits ) - 1 );
		}
	}

	return value;
}

/*
========================
idBitMsg::ReadString
========================
*/
int idBitMsg::ReadString( char * buffer, int bufferSize ) const {
	int l = 0;
	int c;
	while ( 1 ) {
		c = ReadByte();
		if ( c <= 0 || c >= 255 ) {
			break;
		}
		// translate all fmt spec to avoid crash bugs in string routines
		if ( c == '%' ) {
			c = '.';
		}
		if ( c > 127 ) {
			c = '.';
		}

		// we will read past any excessively long string, so
		// the following data can be read, but the string will
		// be truncated
		if ( l < bufferSize - 1 ) {
			buffer[l] = c;
			l++;
		}
	}
	
	buffer[l] = 0;
	return l;
}

/*
========================
idBitMsg::ReadString
========================
*/
int idBitMsg::ReadString( idStr & str ) const {
	char buffer[8192];
	const int length = ReadString( buffer, sizeof( buffer ) );
	str = buffer;
	return length;
}

const char *idBitMsg::ReadString() const {
	ReadString( stringBuffer, sizeof( stringBuffer ) );
	return stringBuffer;
}

const char *idBitMsg::ReadStringLine() const {
	int length = 0;
	while ( length < static_cast<int>( sizeof( stringBuffer ) ) - 1 ) {
		const int c = ReadByte();
		if ( c < 0 || c == 0 || c == '\n' ) break;
		stringBuffer[length++] = c == '%' ? '.' : static_cast<char>( c );
	}
	stringBuffer[length] = '\0';
	return stringBuffer;
}

/*
========================
idBitMsg::ReadData
========================
*/
int idBitMsg::ReadData( void *data, int length ) const {
	if ( oob ) {
		ReadByteAlign();
		const int available = curSize > readCount ? curSize - readCount : 0;
		const int count = length < available ? length : available;
		if ( data && count > 0 ) memcpy( data, readData + readCount, count );
		readCount += count;
		return count;
	}
	byte *bytes = static_cast<byte *>( data );
	for ( int i = 0; i < length; ++i ) {
		const int c = ReadByte();
		if ( c < 0 ) return i;
		if ( bytes ) bytes[i] = static_cast<byte>( c );
	}
	return length;
}

/*
========================
idBitMsg::ReadDeltaDict
========================
*/
bool idBitMsg::ReadDeltaDict( idDict &dict, const idDict *base ) const {
	char		key[MAX_STRING_CHARS];
	char		value[MAX_STRING_CHARS];
	bool		changed = false;

	if ( base != NULL ) {
		dict = *base;
	} else {
		dict.Clear();
	}

	while( ReadString( key, sizeof( key ) ) != 0 ) {
		ReadString( value, sizeof( value ) );
		dict.Set( key, value );
		changed = true;
	}

	while( ReadString( key, sizeof( key ) ) != 0 ) {
		dict.Delete( key );
		changed = true;
	}

	return changed;
}

/*
========================
idBitMsg::DirToBits
========================
*/
int idBitMsg::DirToBits( const idVec3 &dir, int numBits ) {
	int max, bits;
	float bias;

	assert( numBits >= 6 && numBits <= 32 );
	assert( dir.LengthSqr() - 1.0f < 0.01f );

	numBits /= 3;
	max = ( 1 << ( numBits - 1 ) ) - 1;
	bias = 0.5f / max;

	bits = IEEE_FLT_SIGNBITSET( dir.x ) << ( numBits * 3 - 1 );
	bits |= ( idMath::Ftoi( ( idMath::Fabs( dir.x ) + bias ) * max ) ) << ( numBits * 2 );
	bits |= IEEE_FLT_SIGNBITSET( dir.y ) << ( numBits * 2 - 1 );
	bits |= ( idMath::Ftoi( ( idMath::Fabs( dir.y ) + bias ) * max ) ) << ( numBits * 1 );
	bits |= IEEE_FLT_SIGNBITSET( dir.z ) << ( numBits * 1 - 1 );
	bits |= ( idMath::Ftoi( ( idMath::Fabs( dir.z ) + bias ) * max ) ) << ( numBits * 0 );
	return bits;
}

/*
========================
idBitMsg::BitsToDir
========================
*/
idVec3 idBitMsg::BitsToDir( int bits, int numBits ) {
	static float sign[2] = { 1.0f, -1.0f };
	int max;
	float invMax;
	idVec3 dir;

	assert( numBits >= 6 && numBits <= 32 );

	numBits /= 3;
	max = ( 1 << ( numBits - 1 ) ) - 1;
	invMax = 1.0f / max;

	dir.x = sign[( bits >> ( numBits * 3 - 1 ) ) & 1] * ( ( bits >> ( numBits * 2 ) ) & max ) 
					* invMax;

	dir.y = sign[( bits >> ( numBits * 2 - 1 ) ) & 1] * ( ( bits >> ( numBits * 1 ) ) & max ) 
					* invMax;

	dir.z = sign[( bits >> ( numBits * 1 - 1 ) ) & 1] * ( ( bits >> ( numBits * 0 ) ) & max ) 
					* invMax;

	dir.NormalizeFast();
	return dir;
}
