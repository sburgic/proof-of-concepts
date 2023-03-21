/**
 ** Name
 **   main.c
 **
 ** Purpose
 **   Main application
 **
 ** Revision
 **   10-Oct-2020 (SSB) [] Initial
 **/

#include "main.h"

#include "adc.h"
#include "cli.h"
#include "display.h"
#include "gpio.h"
#include "ptypes.h"
#include "tim.h"
#include "uart.h"
#include "state_machine.h"
#include "system_init.h"

#include <stdio.h>

static void critical_error_handler( void )
{
    sm_set_state( STATE_MACHINE_ERROR );

    for(;;);
}

int main( void )
{
    status_t  ret;
    Adc_Rms_t rms[2] = {0};
    bool_t    rms_start_calc;

    system_clk_cfg();
    HAL_Init();
    gpio_init();

    ret  = tmr_bsp_init();
    ret |= tmr_ms_init();

    if ( STATUS_OK != ret )
    {
        critical_error_handler();
    }

    wait( 100, TIME_MSEC );

    ret = uart_init( UART_TO_PC, 115200 );
    if ( STATUS_OK != ret )
    {
        critical_error_handler();
    }
    else
    {
        printf( "\r\nInfo: VAMeter version %d.%d built on %s %s.\r\n"
              , APP_VER_MAJOR
              , APP_VER_MINOR
              , __DATE__
              , __TIME__
              );
        printf( "Info: System Core Clock is %lu MHz.\r\n"
              , SystemCoreClock / 1000000
              );
    }

    if ( STATUS_OK != ret )
    {
        critical_error_handler();
    }

    GPIO_PinState user_pb_state;

    /* Since PB is manually pressed during the power-on cycle there is no need
     * for full debouncing or usage of interrupt routine to read the state.
     */
    user_pb_state = HAL_GPIO_ReadPin( CLI_PB_PORT, CLI_PB_PIN );

    if ( GPIO_PIN_RESET == user_pb_state )
    {
        /* Do a primitive debouncing */
        wait( 75, TIME_MSEC );

        if ( GPIO_PIN_RESET == user_pb_state )
        {
            printf( "Info: Command Line Interface loaded.\r\n\r\n" );
            sm_set_state( STATE_MACHINE_CLI );
            cli_process();
        }
    }
    else
    {
        ret  = adc_init();
        ret |= adc_start();
    }

    if ( STATUS_OK != ret )
    {
        critical_error_handler();
    }

    /* Evaluate RMS over 128 collected samples.
     * As we are sensing the DC only the number of samples in choosen random.
     * For the AC sensing, an exact number of samples has to be provided.
     * Current ADC configuration samples at 2,4 KS/s.
     */
    rms[0].req_samples = 128;
    rms[1].req_samples = 128;

    for(;;)
    {
        rms_start_calc = adc_get_rms_flag();

        if ( FALSE != rms_start_calc )
        {
            adc_calc_rms( rms );
            adc_set_rms_flag( FALSE );
        }
    }

    return 0;
}
