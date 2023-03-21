/**
 ** Name
 **   system_init.c
 **
 ** Purpose
 **   System configuration
 **
 ** Revision
 **   10-Oct-2020 (SSB) [] Initial
 **/

#include "system_init.h"

void system_clk_cfg( void )
{
    RCC_ClkInitTypeDef       clkinitstruct   = {0};
    RCC_OscInitTypeDef       oscinitstruct   = {0};
    RCC_PeriphCLKInitTypeDef prepiphclkstruct = {0};

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    oscinitstruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    oscinitstruct.HSEState       = RCC_HSE_ON;
    oscinitstruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    oscinitstruct.PLL.PLLState   = RCC_PLL_ON;
    oscinitstruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    oscinitstruct.PLL.PLLMUL     = RCC_PLL_MUL3;

    if ( HAL_OK != HAL_RCC_OscConfig(&oscinitstruct))
    {
        while( 1 );
    }

    clkinitstruct.ClockType      = ( RCC_CLOCKTYPE_SYSCLK  \
                                   | RCC_CLOCKTYPE_HCLK    \
                                   | RCC_CLOCKTYPE_PCLK1   \
                                   | RCC_CLOCKTYPE_PCLK2   \
                                   );
    clkinitstruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clkinitstruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
    clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if ( HAL_OK != HAL_RCC_ClockConfig( &clkinitstruct, FLASH_LATENCY_0 ))
    {
        while( 1 );
    }

    prepiphclkstruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    prepiphclkstruct.AdcClockSelection    = RCC_ADCPCLK2_DIV2;

    if ( HAL_OK != HAL_RCCEx_PeriphCLKConfig( &prepiphclkstruct ))
    {
        while( 1 );
    }
}

void HAL_MspInit( void )
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}
