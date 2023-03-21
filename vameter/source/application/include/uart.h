/**
 ** Name
 **   uart.h
 **
 ** Purpose
 **   UART routines
 **
 ** Revision
 **   27-Aug-2020 (SSB) [] Initial
 **/

#ifndef __UART_H__
#define __UART_H__

#include "ptypes.h"

#include <stm32f1xx_hal.h>

#include <stdio.h>

#define UART_TO_PC USART1

#define UART_BUFFER_SIZE      (1024)
#define UART_STRING_DELIMITER ((uint8_t)'\n')

#define UART_WRITE_DATA(UARTx, data) ((UARTx)->DR = (data))
#define UART_READ_DATA(UARTx)        ((UARTx)->DR)

#define UART_TXEMPTY(UARTx) ((UARTx)->SR & USART_FLAG_TXE)
#define UART_WAIT(UARTx)    while (!UART_TXEMPTY(UARTx))

status_t uart_init( USART_TypeDef* uart, uint32_t baudrate );
void uart_puts( USART_TypeDef* uart, char* str );
void uart_send( USART_TypeDef* uart, uint8_t* data, uint16_t count );
uint8_t uart_getc( USART_TypeDef* uart );
uint16_t uart_gets( USART_TypeDef* uart, char* data, uint16_t buff_size );
int16_t uart_find_char( USART_TypeDef* uart, uint8_t ch );
bool_t uart_buff_empty( USART_TypeDef* uart );
bool_t uart_buff_full( USART_TypeDef* uart );
uint16_t uart_buff_count( USART_TypeDef* uart );
void uart_clear_buff( USART_TypeDef* uart );
void uart_set_custom_string_delimiter( USART_TypeDef* uart, uint8_t delim );
int16_t uart_find_string( USART_TypeDef* uart, char* str );

#endif /* __UART_H__ */
