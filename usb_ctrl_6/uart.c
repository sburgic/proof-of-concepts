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

#include "uart.h"

#include <pic.h>

void uart_init( void )
{
    TX9   = 0; /* 8-bit transmission */
    TXEN  = 1; /* Transmit enable */
    SYNC  = 0; /* Asynchronous mode */
    BRGH  = 1; /* High speed mode */

    SPEN  = 1; /* Serial port enable */
    RX9   = 0; /* 8-bit reception */
    CREN  = 1; /* Continuous receive enable */

    SPBRG = 25; /* 9600 bps with XTAL 4MHz */
    BRG16 = 0;

    WUE   = 0;
    ABDEN = 0;

    RCIE  = 1;  /* Receive interrupt enable */
    TXIE  = 0;  /* Transmit interrupt disable */
}

void uart_putch( uint8_t ch )
{
    while ( !TRMT );
    TXREG = ch;
}

void uart_puts( uint8_t* buff, uint16_t len )
{
    uint16_t i ;

    for ( i = 0; i < len; i++ )
    {
        uart_putch( buff[i] );
    }
}
