/**
  ******************************************************************************
  * @file    application/src/stm32f1xx_hal_msp.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    01-Jun-2017
  * @brief   HAL MSP module.
  *****************************************************************************
 */

#include "main.h"
#include "stm32f1xx_hal_msp.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f100xb.h"

HAL_Ret hal_msp_uart_init ( UART_Base base )
{
    HAL_Ret         ret  = HAL_OK;
    HAL_GPIO_Data   gpio = {0};

    if ( UART1 == base )
    {
        /* Enable GPIO clocks */
        __HAL_RCC_GPIOA_CLK_ENABLE();

           /* Enable UART1 clock */
        __HAL_RCC_USART1_CLK_ENABLE();

        /* Tx pin configuration */
        gpio.Pin    = UART1_TX_PIN;
        gpio.Mode   = GPIO_MODE_AF_PP;
        gpio.Pull   = GPIO_PULLUP;
        gpio.Speed  = GPIO_SPEED_FREQ_HIGH;

        /* Initialize Tx pin */
        HAL_GPIO_Init ( UART1_TX_GPIO_PORT, &gpio );

        /* Rx pin configuration */
        gpio.Pin    = UART1_RX_PIN;
        gpio.Mode   = GPIO_MODE_INPUT;

        /* Initialize Rx pin */
        HAL_GPIO_Init ( UART1_RX_GPIO_PORT, &gpio );
    }
    else if ( UART2 == base )
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_USART2_CLK_ENABLE();

        gpio.Pin    = UART2_TX_PIN;
        gpio.Mode   = GPIO_MODE_AF_PP;
        gpio.Pull   = GPIO_PULLUP;
        gpio.Speed  = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init ( UART2_TX_GPIO_PORT, &gpio );

        gpio.Pin    = UART2_RX_PIN;
        gpio.Mode   = GPIO_MODE_INPUT;

        HAL_GPIO_Init ( UART2_RX_GPIO_PORT, &gpio );
    }
    else
    {
        ret = HAL_ERROR;
    }

    return ret;
}


void hal_msp_bsp_tmr_init ( void )
{
    TMR_MASTER_CLK_ENALBE();
    TMR_SLAVE_CLK_ENABLE();
}


void hal_msp_ms_tmr_init ( void )
{
    TMR_MS_CLK_ENABLE();
}

void hal_msp_relay_init ( void )
{
    HAL_GPIO_Data gpio = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio.Pin   = RELAY_PIN;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;

    HAL_GPIO_Init( RELAY_GPIO_PORT, &gpio );

    HAL_GPIO_WritePin( RELAY_GPIO_PORT
                     , RELAY_PIN
                     , GPIO_PIN_RESET
                     );
}

void hal_msp_relay_set ( GPIO_PinState new_state )
{
    if ( GPIO_PIN_SET != new_state )
    {
        HAL_GPIO_WritePin( RELAY_GPIO_PORT
                         , RELAY_PIN
                         , GPIO_PIN_RESET
                         );
    }
    else
    {
        HAL_GPIO_WritePin( RELAY_GPIO_PORT
                         , RELAY_PIN
                         , GPIO_PIN_SET
                         );
    }
}
