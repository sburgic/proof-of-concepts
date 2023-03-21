/**
 ** Name
 **   uart.h
 **
 ** Purpose
 **   UART API
 **
 ** Revision
 **   06-Feb-2021 (SSB) [] Initial
 **/

#ifndef __UART_H__
#define __UART_H__

#include "ptypes.h"

#define UART_STRING_DELIMITER ((uint8_t)'\n')

void uart_init( void );
void uart_putch( uint8_t ch );
void uart_puts( uint8_t* buff, uint16_t len );

#endif /* __UART_H__ */
