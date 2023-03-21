/**
 ** Name
 **   main.c
 **
 ** Purpose
 **   Main application
 **
 ** Revision
 **   21-Apr-2021 (SSB) [] Initial
 **/

#include "ptypes.h"
#include "system_init.h"
#include "time.h"

#define OUT1 GPIO_PIN_14
#define OUT2 GPIO_PIN_15
#define OUT3 GPIO_PIN_12
#define OUT4 GPIO_PIN_13

#define MAX_STEPS 4
#define WAIT_MAX  1800
#define WAIT_MIN  1050
#define WAIT_STEP 25

volatile bool_t pb1;
volatile bool_t pb2;
bool_t          init_done = FALSE;

static void drive( uint8_t step )
{
    switch ( step )
    {
        case 0:
            HAL_GPIO_WritePin( GPIOB, OUT1, GPIO_PIN_SET );
            HAL_GPIO_WritePin( GPIOB, OUT2, GPIO_PIN_SET );
            HAL_GPIO_WritePin( GPIOB, OUT3, GPIO_PIN_RESET );
            HAL_GPIO_WritePin( GPIOB, OUT4, GPIO_PIN_RESET );
        break;
        case 1:
            HAL_GPIO_WritePin( GPIOB, OUT1, GPIO_PIN_RESET );
            HAL_GPIO_WritePin( GPIOB, OUT2, GPIO_PIN_SET );
            HAL_GPIO_WritePin( GPIOB, OUT3, GPIO_PIN_SET );
            HAL_GPIO_WritePin( GPIOB, OUT4, GPIO_PIN_RESET );
        break;
        case 2:
            HAL_GPIO_WritePin( GPIOB, OUT1, GPIO_PIN_RESET );
            HAL_GPIO_WritePin( GPIOB, OUT2, GPIO_PIN_RESET );
            HAL_GPIO_WritePin( GPIOB, OUT3, GPIO_PIN_SET );
            HAL_GPIO_WritePin( GPIOB, OUT4, GPIO_PIN_SET );
        break;
        case 3:
            HAL_GPIO_WritePin( GPIOB, OUT1, GPIO_PIN_SET );
            HAL_GPIO_WritePin( GPIOB, OUT2, GPIO_PIN_RESET );
            HAL_GPIO_WritePin( GPIOB, OUT3, GPIO_PIN_RESET );
            HAL_GPIO_WritePin( GPIOB, OUT4, GPIO_PIN_SET );
        break;
        default:
            HAL_GPIO_WritePin( GPIOB, OUT1, GPIO_PIN_RESET );
            HAL_GPIO_WritePin( GPIOB, OUT2, GPIO_PIN_RESET );
            HAL_GPIO_WritePin( GPIOB, OUT3, GPIO_PIN_RESET );
            HAL_GPIO_WritePin( GPIOB, OUT4, GPIO_PIN_RESET );
        break;
    }
}

int main( void )
{
    int8_t   		 i;
    uint8_t  		 spin      = 0xFF;
    Bsp_Time 		 wait      = WAIT_MAX;
    Bsp_Time 		 wait_step = WAIT_STEP;
	GPIO_InitTypeDef gpio 	   = {0};

    system_clk_cfg();
    HAL_Init();
    bsp_tmr_init();
    bsp_sched_tmr_init();

    bsp_wait( 100, BSP_TIME_MSEC );

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio.Pin   = OUT1 | OUT2 | OUT3 | OUT4;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOB, &gpio );

    gpio.Pin   = GPIO_PIN_10;
    gpio.Mode  = GPIO_MODE_INPUT;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( GPIOA, &gpio );
    HAL_GPIO_Init( GPIOB, &gpio );

    bsp_wait( 100, BSP_TIME_MSEC );

    init_done = TRUE;

    for (;;)
    {
        if ( FALSE != pb1 )
        {
            if ( 0xFF == spin )
            {
                spin = 1;
            }
            else if ( 1 == spin )
            {
                wait -= wait_step;
            }
            else if ( 2 == spin )
            {
                wait += wait_step;
            }

            pb1 = FALSE;
        }

        if ( FALSE != pb2 )
        {
            if ( 0xFF == spin )
            {
                spin = 2;
            }

            else if ( 2 == spin )
            {
                wait -= wait_step;
            }
            else if ( 1 == spin )
            {
                wait += wait_step;
            }

            pb2 = FALSE;
        }

        if ( wait < WAIT_MIN )
        {
            wait = WAIT_MIN;
        }
        else if ( wait > WAIT_MAX )
        {
            wait = WAIT_MAX;
            spin = 0xFF;
        }

        if ( 1 == spin )
        {
            for ( i = 0; i < MAX_STEPS; i++ )
            {
                drive( i );
                bsp_wait( wait, BSP_TIME_USEC );
            }
        }
        else if ( 2 == spin )
        {
            for ( i = ( MAX_STEPS - 1 ); i >= 0; i-- )
            {
                drive( i );
                bsp_wait( wait, BSP_TIME_USEC );
            }
        }
        else
        {
            drive( 0xFF );
        }
    }

#ifndef __ICCARM__
    return 0;
#endif
}
