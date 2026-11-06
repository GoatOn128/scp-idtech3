#pragma hdrstop
#include "../precompiled.h"
#include "CRC16.h"

static constexpr uint16 CRC16_INIT_VALUE = 0xffff;
static constexpr uint16 CRC16_XOR_VALUE = 0x0000;

static uint16 CRC16_UpdateByte( uint16 crc, byte data ) {
	crc ^= static_cast<uint16>( data ) << 8;
	for ( int bit = 0; bit < 8; ++bit ) {
		crc = ( crc & 0x8000 ) ? static_cast<uint16>( ( crc << 1 ) ^ 0x1021 ) : static_cast<uint16>( crc << 1 );
	}
	return crc;
}

void CRC16_InitChecksum( uint16 &crcvalue ) {
	crcvalue = CRC16_INIT_VALUE;
}

void CRC16_UpdateChecksum( uint16 &crcvalue, const void *data, int length ) {
	const byte *bytes = static_cast<const byte *>( data );
	while ( length-- > 0 ) {
		crcvalue = CRC16_UpdateByte( crcvalue, *bytes++ );
	}
}

void CRC16_FinishChecksum( uint16 &crcvalue ) {
	crcvalue ^= CRC16_XOR_VALUE;
}

uint16 CRC16_BlockChecksum( const void *data, int length ) {
	uint16 crcvalue;
	CRC16_InitChecksum( crcvalue );
	CRC16_UpdateChecksum( crcvalue, data, length );
	CRC16_FinishChecksum( crcvalue );
	return crcvalue;
}
