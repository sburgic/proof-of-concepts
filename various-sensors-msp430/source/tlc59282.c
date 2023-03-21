/**
 ** Name
 **   tlc59282.c
 **
 ** Purpose
 **   TLC59282 driver
 **
 ** Revision
 **   07-Dec-2020 (SSB) [] Initial
 **/

#include "tlc59282.h"

#include "system.h"

#define TLC59282_SPI_LATCH_LOW()   do { P3OUT &= ~BIT1; } while (0)
#define TLC59282_SPI_LATCH_HIGH()  do { P3OUT |= BIT1; } while (0)
#define TLC59282_SPI_SEND_BYTE(x)  do { UCA1IFG &= ~UCRXIFG; UCA1TXBUF= (x); } \
                                      while (0)
#define TLC59282_SPI_WAIT_DONE()   while (!(UCA1IFG & UCRXIFG))

#define SEG_A ((uint32_t)0x01)
#define SEG_B ((uint32_t)0x02)
#define SEG_C ((uint32_t)0x04)
#define SEG_D ((uint32_t)0x08)
#define SEG_E ((uint32_t)0x10)
#define SEG_F ((uint32_t)0x20)
#define SEG_G ((uint32_t)0x40)
#define SEG_H ((uint32_t)0x80)

/* ASCII lookup table */
static const uint8_t tlc59282_ascii_tbl[256] =
{
    [32] = 0x00,  /* Space */

    [45] = SEG_G, /* Minus */

    [48]  = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,          /* 0 */
    [49]  = SEG_B | SEG_C,                                          /* 1 */
    [50]  = SEG_A | SEG_B | SEG_G | SEG_D | SEG_E,                  /* 2 */
    [51]  = SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                  /* 3 */
    [52]  = SEG_F | SEG_G | SEG_B | SEG_C,                          /* 4 */
    [53]  = SEG_A | SEG_F | SEG_C | SEG_D | SEG_G,                  /* 5 */
    [54]  = SEG_A | SEG_F | SEG_C | SEG_D | SEG_G | SEG_E,          /* 6 */
    [55]  = SEG_A | SEG_B | SEG_C,                                  /* 7 */
    [56]  = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,  /* 8 */
    [57]  = SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,          /* 9 */

    [58]  = SEG_H,                                                  /* : */

    [69]  = SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,                  /* E */
    [83]  = SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                  /* S */
    [99]  = SEG_D | SEG_E | SEG_G,                                  /* c */
    [100] = SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,                  /* d */
    [114] = SEG_E | SEG_G,                                          /* r */
    [117] = SEG_C | SEG_D | SEG_E,                                  /* u */

};

static __inline__ void tlc59282_latch( void )
{
    TLC59282_SPI_LATCH_HIGH();
    wait( 1 );
    TLC59282_SPI_LATCH_LOW();
}

static void tlc59282_send_byte( uint8_t byte )
{
    TLC59282_SPI_SEND_BYTE( byte );
    TLC59282_SPI_WAIT_DONE();
}

void tlc59282_init( void )
{
    UCA1CTLW0 |= UCSWRST;  /* Software reset */

    UCA1CTLW0 |= UCSYNC;   /* Synchronous mode */
    UCA1CTLW0 |= UCCKPH;   /* Data is captured on the first UCLK edge */
    UCA1CTLW0 &= ~UCCKPL;  /* Clock Polarity 0 */
    UCA1CTLW0 |= UCMSB;    /* MSB first */
    UCA1CTLW0 &= ~UC7BIT;  /* 8-bit data */
    UCA1CTLW0 |= UCMST;    /* Master mode */
    UCA1CTLW0 |= UCMODE_0; /* 3-pin SPI; Manual CS control */
    UCA1CTLW0 |= UCSSEL_2; /* 8 MHz clock (from SMCLK) */

    UCA1BRW = 2; /* Prescaler set to 2 */

    P2SEL0 |= ( BIT4 + BIT5 + BIT6 );
    P3OUT  |= ( BIT1 + BIT3 );
    P3DIR  |= BIT1;
    P2DIR  |= ( BIT3 + BIT4 + BIT6 );
    P2DIR  &= BIT5;

    UCA1CTLW0 &= ~UCSWRST; /* Release from reset */
}

void tlc59282_send_uint32( uint32_t data )
{
    uint8_t  array[4];
    uint8_t  i;

    array[3] = ( data & 0xFF );
    array[2] = ((data >> 8) & 0xFF );
    array[1] = ((data >> 16) & 0xFF );
    array[0] = ((data >> 24) & 0xFF );

    for ( i = 0; i < 4; i++ )
    {
         tlc59282_send_byte( array[i] );
    }

    tlc59282_latch ();
}

void tlc59282_send_uint32_nl( uint32_t data )
{
    uint8_t  array[4];
    uint8_t  i;

    array[3] = ( data & 0xFF );
    array[2] = ((data >> 8) & 0xFF );
    array[1] = ((data >> 16) & 0xFF );
    array[0] = ((data >> 24) & 0xFF );

    for ( i = 0; i < 4; i++ )
    {
        tlc59282_send_byte ( array[i] );
    }
}

void tlc59282_send_uint16( uint16_t data )
{
    uint8_t  array[2];
    uint8_t  i;

    array[1] = ( data & 0xFF );
    array[0] = ((data >> 8) & 0xFF );

    for ( i = 0; i < 2; i++ )
    {
         tlc59282_send_byte( array[i] );
    }

    tlc59282_latch();
}

void tlc59282_led_puts( uint8_t* str, uint8_t size )
{
    uint8_t  i;
    uint8_t  byte;
    uint8_t  skipped = 0;

    if ( NULL != str )
    {
        for ( i = 0; i < size; i++ )
        {
            byte = (uint8_t) str[i];

            if (( i < ( size - 1 )) && ( '.' == str[i + 1]))
            {
                tlc59282_send_byte( tlc59282_ascii_tbl[byte] | SEG_H );
            }
            else if ( '.' != byte )
            {
                tlc59282_send_byte( tlc59282_ascii_tbl[byte] );
            }
            else
            {
                skipped++;
            }
        }

        /* Clear till end */
        for ( i = ( size - skipped ); i < TLC_NO_OF_SEG; i++ )
        {
            tlc59282_send_byte( tlc59282_ascii_tbl[32] ); /* Space */
        }
    }

    tlc59282_latch();
}

void tlc59282_led_test( void )
{
    tlc59282_send_uint32_nl( SEG_A
                           | ( SEG_A << 8 )
                           | ( SEG_A << 16 )
                           | ( SEG_A << 24 )
                           );
    tlc59282_send_uint16( SEG_A | ( SEG_A << 8 ));
    wait( 500000 );

    tlc59282_send_uint32_nl( SEG_B
                           | ( SEG_B << 8 )
                           | ( SEG_B << 16 )
                           | ( SEG_B << 24 )
                           );
    tlc59282_send_uint16( SEG_B | ( SEG_B << 8 ));
    wait( 500000 );


    tlc59282_send_uint32_nl( SEG_C
                           | ( SEG_C << 8 )
                           | ( SEG_C << 16 )
                           | ( SEG_C << 24 )
                           );
    tlc59282_send_uint16( SEG_C | ( SEG_C << 8 ));
    wait( 500000 );

    tlc59282_send_uint32_nl( SEG_D
                           | ( SEG_D << 8 )
                           | ( SEG_D << 16 )
                           | ( SEG_D << 24 )
                           );
    tlc59282_send_uint16( SEG_D | ( SEG_D << 8 ));
    wait( 500000 );

    tlc59282_send_uint32_nl( SEG_E
                           | ( SEG_E << 8 )
                           | ( SEG_E << 16 )
                           | ( SEG_E << 24 )
                           );
    tlc59282_send_uint16( SEG_E | ( SEG_E << 8 ));
    wait( 500000 );

    tlc59282_send_uint32_nl(   SEG_F
                           | ( SEG_F << 8 )
                           | ( SEG_F << 16 )
                           | ( SEG_F << 24 )
                           );
    tlc59282_send_uint16( SEG_F | ( SEG_F << 8 ));
    wait( 500000 );

    tlc59282_send_uint32_nl(   SEG_G
                           | ( SEG_G << 8 )
                           | ( SEG_G << 16 )
                           | ( SEG_G << 24 )
                           );
    tlc59282_send_uint16(   SEG_G | ( SEG_G << 8 ));
    wait( 500000 );

    tlc59282_send_uint32_nl( SEG_H
                           | ( SEG_H << 8 )
                           | ( SEG_H << 16 )
                           | ( SEG_H << 24 )
                           );
    tlc59282_send_uint16( SEG_H | ( SEG_H << 8 ));
    wait( 500000 );

    tlc59282_send_uint32_nl  ( 0xFFFFFFFF );
    tlc59282_send_uint16     ( 0xFFFF );
    wait( 1000000 );

    tlc59282_clear();
}

void tlc59282_clear( void )
{
    uint8_t  i;

    for ( i = 0; i < TLC_NO_OF_SEG; i++ )
    {
         tlc59282_send_byte( tlc59282_ascii_tbl[0] );
    }

    tlc59282_latch();
}
