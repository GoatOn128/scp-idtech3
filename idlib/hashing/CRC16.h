#ifndef __CRC16_H__
#define __CRC16_H__

#include "../sys/sys_types.h"

void CRC16_InitChecksum( uint16 &crcvalue );
void CRC16_UpdateChecksum( uint16 &crcvalue, const void *data, int length );
void CRC16_FinishChecksum( uint16 &crcvalue );
uint16 CRC16_BlockChecksum( const void *data, int length );

#endif
