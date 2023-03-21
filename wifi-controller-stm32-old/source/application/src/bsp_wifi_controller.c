/**
  ******************************************************************************
  * @file    bsp/bsp_wifi_controller/bsp_wifi_controller.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.1
  * @date    25-Mar-2019
  * @brief   This file provides
  *            - set of firmware functions to manage LED and push-button
  *              on WIFI CONTROLLER board.
  ******************************************************************************
  */

#include "bsp_wifi_controller.h"
#include "types.h"
#include "sl_bit.h"
#include "sl_string.h"
#include "main.h"
#include "stm32f1xx_hal_msp.h"

/**
* @brief WIFI CONTROLLER BSP version number
*/
#define __STM32F100_WIFI_CTRL_BSP_VERSION_MAIN   (0x01) /**< [31:24]
                                                        *   main version
                                                        */

#define __STM32F100_WIFI_CTRL_BSP_VERSION_SUB1   (0x00) /**< [23:16]
                                                        *   sub1 version
                                                        */

#define __STM32F100_WIFI_CTRL_BSP_VERSION_SUB2   (0x00) /**< [15:8]
                                                        *   sub2 version
                                                        */

#define __STM32F100_WIFI_CTRL_BSP_VERSION_RC     (0x00) /**< [7:0]
                                                        *    release candidate
                                                        */

#define __STM32F100_WIFI_CTRL_BSP_VERSION\
                    ((__STM32F100_WIFI_CTRL_BSP_VERSION_MAIN << 24)\
                    |(__STM32F100_WIFI_CTRL_BSP_VERSION_SUB1 << 16)\
                    |(__STM32F100_WIFI_CTRL_BSP_VERSION_SUB2 << 8 )\
                    |(__STM32F100_WIFI_CTRL_BSP_VERSION_RC))

#define BSP_LIVE_CHECK_MS                       ((int16_t)2000)
#define BSP_LIVE_LED_ON_MS                      ((int16_t)50)

GPIO_TypeDef*   LED_PORT[NUM_OF_LEDS] = { LED_STATUS_GPIO_PORT
                                        };

const uint16_t  LED_PIN[NUM_OF_LEDS]  = { LED_STATUS_PIN
                                        };

GPIO_TypeDef*   BUTTON_PORT[NUM_OF_BUTTONS]  = { S2_BUTTON_GPIO_PORT };
const uint16_t  BUTTON_PIN[NUM_OF_BUTTONS]   = { S2_BUTTON_PIN };
const uint8_t   BUTTON_IRQn[NUM_OF_BUTTONS]  = { S2_BUTTON_EXTI_IRQn };

uint32_t bsp_get_version ( void )
{
    return __STM32F100_WIFI_CTRL_BSP_VERSION;
}

uint16_t bsp_print_version ( uint8_t* buff )
{
    uint32_t bsp_ver;
    uint16_t len = 0;

    bsp_ver = bsp_get_version ();

    len = sl_sprintf_s ( buff
                       , (uint8_t*)"%s"
                       , (uint8_t*)"Current BSP Version:\r\n"
                       , SL_MAX_STRING_SIZE
                       );
    len += sl_sprintf_x ( &buff[len]
                        , (uint8_t*)"\tMain: 0x%02x\r\n"
                        , (uint32_t) BITMASK_GET( bsp_ver, 31, 24 )
                        , (SL_MAX_STRING_SIZE - len )
                        );
    len += sl_sprintf_x ( &buff[len]
                        , (uint8_t*)"\tSub1: 0x%02x\r\n"
                        , (uint32_t) BITMASK_GET( bsp_ver, 23, 16 )
                        , (SL_MAX_STRING_SIZE - len )
                        );
    len += sl_sprintf_x ( &buff[len]
                        , (uint8_t*)"\tSub2: 0x%02x\r\n"
                        , (uint32_t) BITMASK_GET( bsp_ver, 15, 8 )
                        , (SL_MAX_STRING_SIZE - len )
                        );
    len += sl_sprintf_x ( &buff[len]
                        , (uint8_t*)"\tRC:   0x%02x\r\n"
                        , (uint32_t) BITMASK_GET( bsp_ver, 7, 0 )
                        , (SL_MAX_STRING_SIZE - len )
                        );
    return len;
}


void bsp_led_init ( BSP_LED led )
{
    GPIO_InitTypeDef  gpioinitstruct = {0};

    /* Enable the GPIO_LED Clock */
    LEDx_GPIO_CLK_ENABLE ( led );

    /* Configure the GPIO_LED pin */
    gpioinitstruct.Pin    = LED_PIN[led];
    gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpioinitstruct.Pull   = GPIO_NOPULL;
    gpioinitstruct.Speed  = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init ( LED_PORT[led], &gpioinitstruct );

    /* Reset PIN to switch off the LED */
    HAL_GPIO_WritePin ( LED_PORT[led], LED_PIN[led], GPIO_PIN_RESET );
}


void bsp_led_on ( BSP_LED led )
{
    HAL_GPIO_WritePin ( LED_PORT[led], LED_PIN[led], GPIO_PIN_SET );
}


void bsp_led_off ( BSP_LED led )
{
    HAL_GPIO_WritePin ( LED_PORT[led], LED_PIN[led], GPIO_PIN_RESET );
}


void bsp_led_toggle (BSP_LED led)
{
    HAL_GPIO_TogglePin ( LED_PORT[led], LED_PIN[led] );
}


void bsp_blink_on_error ( BSP_LED led )
{
    while ( TRUE )
    {
        HAL_GPIO_TogglePin ( LED_PORT[led], LED_PIN[led] );
        hal_delay ( 100 );
    }
}


void bsp_blink_alive ( void )
{
    static int16_t  led_time_on = BSP_LIVE_LED_ON_MS;
    static int16_t  app_alive   = BSP_LIVE_CHECK_MS;

    if ( app_alive > ZERO )
    {
        --app_alive;
    }
    else
    {
        if ( led_time_on > ZERO )
        {
            --led_time_on;
            bsp_led_on ( LED_STATUS );
        }
        else
        {
            bsp_led_off ( LED_STATUS );
            app_alive = BSP_LIVE_CHECK_MS;
            led_time_on = BSP_LIVE_LED_ON_MS;
        }
    }
}

void bsp_pb_init ( BSP_Button button, BSP_Button_Mode mode )
{
    GPIO_InitTypeDef gpioinitstruct = {0};

    /* Enable the BUTTON Clock */
    // BUTTONx_GPIO_CLK_ENABLE ( button );
    S2_BUTTON_GPIO_CLK_ENABLE();

    gpioinitstruct.Pin    = BUTTON_PIN[button];
    gpioinitstruct.Pull   = GPIO_PULLUP;
    gpioinitstruct.Speed  = GPIO_SPEED_FREQ_HIGH;

    if ( BUTTON_MODE_GPIO == mode )
    {
        /* Configure Button pin as input */
        gpioinitstruct.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init ( BUTTON_PORT[button], &gpioinitstruct );
    }
    else if ( BUTTON_MODE_EXTI == mode )
    {
        /* Configure Button pin as input with External interrupt */
        gpioinitstruct.Mode = GPIO_MODE_IT_RISING;
        HAL_GPIO_Init ( BUTTON_PORT[button], &gpioinitstruct );

        /* Enable and set Button EXTI Interrupt to the lowest priority */
        HAL_NVIC_SetPriority ( (IRQn_Type)(BUTTON_IRQn[button]), 0x0F, 0 );
        HAL_NVIC_EnableIRQ( (IRQn_Type)(BUTTON_IRQn[button]) );
        
//        HAL_GPIO_LockPin ( BUTTON_PORT[button], BUTTON_PIN[button] );
    }
}

uint32_t bsp_pb_get_state( BSP_Button button )
{
    return HAL_GPIO_ReadPin ( BUTTON_PORT[button], BUTTON_PIN[button] );
}