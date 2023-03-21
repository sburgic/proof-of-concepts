/**
 ** Name
 **   utils.h
 **
 ** Purpose
 **   Utilities
 **
 ** Revision
 **   02-Jan-2021 (SSB) [] Initial
 **/

#include "ptypes.h"

int16_t util_strcmp( uint8_t* s1, uint8_t* s2, uint16_t len );
void util_memcpy( uint8_t* dest, uint8_t* src, uint16_t len );
int16_t util_atoi( uint8_t* str );
void util_reverse( uint8_t* s, uint32_t s_len );
uint32_t util_itoa( int32_t n, uint8_t* s, uint32_t s_max );
