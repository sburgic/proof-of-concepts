/**
 ** Name
 **   main.c
 **
 ** Purpose
 **   Main application
 **
 ** Revision
 **   06-Feb-2021 (SSB) [] Initial
 **/

#include "main.h"

#include "comm.h"
#include "uart.h"
#include "utils.h"

#define RTSRC_HEADER      ((uint8_t*)"RTSRC")
#define RTSRC_HEADER_SIZE (5)
#define RTSRC_ACK         ((uint8_t*)"RTSRCACK\r\n")
#define RTSRC_ACK_SIZE    (10)

static volatile uint8_t* uart_buff       = NULL;;
static volatile uint8_t  buff_cnt        = 0;
static volatile uint8_t  string_received = 0;
static volatile uint16_t elapsed         = 0;

static void pic_init( void )
{
    PORTA  = 0;
    PORTB  = 0;

    ANSELA = 0;
    ANSELB = 0;

    C1ON  = 0;
    C2ON  = 0; 
    TRISA = 0b00000000;
    TRISB = 0b00000010; /* RB1=Rx, RB2=Tx */

    T1CON = 0b01100001; /* Fosc, 1:4 */
    PIE1  = 0b00000001; /* TMR1IE */

    PEIE  = 1;
    GIE   = 1;
}

void main( void )
{
    bool_t   answer;
    uint16_t i;
    int16_t  res;
    bool_t   sv_flag = FALSE;

    pic_init();
    __delay_ms(50);

    uart_init();
    __delay_ms(50);

    uart_buff = comm_get_buff_hdl();

    while ( 1 )
    {
        if ( 0 != string_received )
        {
            res = util_strcmp((uint8_t*) uart_buff
                             , RTSRC_HEADER
                             , RTSRC_HEADER_SIZE
                             );

            if ( 0 == res )
            {
                answer = comm_process_msg( (uint8_t*) uart_buff
                                         , (uint16_t*) &buff_cnt
                                         , RTSRC_HEADER_SIZE
                                         );
                if ( FALSE != answer )
                {
                    uart_puts( RTSRC_HEADER, RTSRC_HEADER_SIZE );
                    uart_puts((uint8_t*) uart_buff, buff_cnt );
                    uart_puts((uint8_t*) "\r\n", 2 );
                }
                else
                {
                    uart_puts( RTSRC_ACK, RTSRC_ACK_SIZE );
                }
                
                elapsed = 0;
            }

            string_received = 0;
            buff_cnt = 0;
        }

        sv_flag = comm_get_sv_state();
        
        if ( FALSE != sv_flag )
        {
            if ( elapsed > 100 )
            {
                for ( i = 0; i < 6; i++ )
                {
                    comm_set_output_state( i + 1, 0 );
                }
                elapsed = 0;
            }    
        }
    }
}

void interrupt ISR( void )
{
    if ( 0 != RCIF )
    {
        uart_buff[buff_cnt] = RCREG;

        if ( UART_STRING_DELIMITER == uart_buff[buff_cnt] )
        {
            string_received = 1;
        }
        else
        {
            buff_cnt++;
        }

        if ( buff_cnt > COMM_BUFF_SIZE )
        {
            buff_cnt = 0;
        }

        RCIF = 0;
    }
    
    if ( 0 != TMR1IF )
    {
        elapsed++;
        TMR1IF = 0;
    }
}
