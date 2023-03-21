/**
  ******************************************************************************
  * @file    application/src/main.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    25-Mar-2019
  * @brief   Main program body
  ******************************************************************************
 */

#include "main.h"

#include "types.h"
#include "stm32f1xx_hal_msp.h"
#include "bsp_wifi_controller.h"
#include "bsp_time.h"
#include "bl_uart.h"
#include "esp8266.h"
#include "ds18b20.h"
#include "cli.h"
#include "lock.h"

#include <sl_string.h>

/* Configure system clock */
static void system_clk_cfg( void );

int main( void )
{
    HAL_Ret           ret;
    Esp_Ret           e_ret;
    bool_t            b_ret;
    Cli_Ret           c_ret;
    Esp_Mode          e_mode;
    uint8_t           out[SL_MAX_STRING_SIZE] = {0};
    DS_18B20_Hdl      ds;
    uint8_t           ds_no_of_dev = OW_NO_DEVICES;
    int16_t           temp;
    uint8_t           no_of_retries = 0;
    Esp_Relay_State_t rly_state;

    ret = hal_init();
    if ( HAL_OK != ret )
    {
        bsp_blink_on_error ( LED_ERROR );
    }

    /* Configure the system clock to 24 MHz */
    system_clk_cfg ();

    bsp_led_init( LED_STATUS );
    bsp_pb_init( S2_BUTTON, BUTTON_MODE_GPIO );
    hal_msp_relay_init();

    ret = bl_uart_init( UART_DBG, UART_DBG_SPEED );

    if ( HAL_OK != ret )
    {
        bsp_blink_on_error( LED_ERROR );
    }
    else
    {
        bsp_print_version( out );

        bl_uart_send( UART_DBG
                    , out
                    , sl_strnlen( out, SL_MAX_STRING_SIZE )
                    );

        bl_uart_send( UART_DBG
                    , (uint8_t*)"HAL initialized.\r\n"
                    , 18
                    );

        bl_uart_send( UART_DBG
                    , (uint8_t*)"UART initialized.\r\n"
                    , 19
                    );
    }

    ret  = bsp_tmr_init ();
    ret |= ms_tmr_init ();

    if( HAL_OK != ret )
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"Timers initialization failed.\r\n"
                    , 31
                    );
        bsp_blink_on_error( LED_ERROR );
    }
    else
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"Timers initialized.\r\n"
                    , 21
                    );
    }

    ds  = ds18b20_get_handle();
    ret = ds18b20_init( ds, DS18B20_GPIO_PORT, DS18B20_PIN );

    if( HAL_OK != ret )
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"1-Wire bus initialization failed.\r\n"
                    , 35
                    );
    }
    else
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"1-Wire bus initialized.\r\n"
                    , 25
                    );

        ds_no_of_dev = ow_list_all_devices( ds );
    }

    ret   = bl_uart_init( UART1, ESP_UART_SPEED );
    e_ret = esp_init( UART1 );

    if (( ESP_RET_OK != e_ret ) && ( HAL_OK == ret ))
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"ESP8266 initialization failed.\r\n"
                    , 32
                    );

        bsp_blink_on_error( LED_ERROR );
    }
    else
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"ESP8266 initialized.\r\n"
                    , 22
                    );
    }

    b_ret = check_device_is_locked();

    if ( FALSE != b_ret )
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"Device is locked!\r\n"
                    , 19
                    );
        
        while ( TRUE )
        {
            bsp_blink_on_error( LED_ERROR );
        }
    }

    b_ret = cli_check_state();

    if ( FALSE != b_ret )
    {
        c_ret = cli_login();

        if ( CLI_RET_OK == c_ret )
        {
            cli_print_info();
            cli_start();
        }
        else
        {
            NVIC_SystemReset();
        }
    }

    e_mode = esp_get_mode();
    if ( ESP_MODE_CLIENT != e_mode )
    {
        e_ret  = esp_set_mode( ESP_MODE_AP );
        e_ret |= esp_set_ssid();
        e_ret |= esp_enable_dhcp();

        if ( ESP_RET_OK == e_ret )
        {
            e_ret = esp_create_tcp_server();
        }
        else
        {
            bl_uart_send( UART_DBG
                        , (uint8_t*)"ESP8266 AP setup failed.\r\n"
                        , 26
                        );
            bsp_blink_on_error( LED_ERROR );
        }

        if ( ESP_RET_OK == e_ret )
        {
            e_mode = esp_get_mode();

            while ( ESP_MODE_CLIENT != e_mode )
            {
                e_ret  = esp_tcp_listen();
                e_mode = esp_get_mode();
            }
        }
    }
    else
    {
        e_ret  = esp_set_mode( ESP_MODE_CLIENT );

        if ( ESP_RET_OK != e_ret )
        {
            bl_uart_send( UART_DBG
                        , (uint8_t*)"ESP8266 Client setup failed.\r\n"
                        , 30
                        );
            bsp_blink_on_error( LED_ERROR );
        }
        else
        {
            do
            {
                e_ret = esp_connect_to_ap( 0, 0 );
                no_of_retries++;
            } while (( ESP_RET_OK != e_ret )
              && ( no_of_retries < ESP_NO_OF_RETRIES ));

            if ( ESP_RET_OK == e_ret )
            {
                bl_uart_send( UART_DBG
                            , (uint8_t*)"Connected to SSID.\r\n"
                            , 20
                            );
            }
            else
            {
                bl_uart_send( UART_DBG
                            , (uint8_t*)"Failed to connect to the SSID.\r\n"
                            , 32
                            );
                bl_uart_send( UART_DBG
                            , (uint8_t*)"Check your network credentials.\r\n"
                            , 33
                            );
                bl_uart_send( UART_DBG
                            , (uint8_t*)"Device will now return "
                                        "to the AP mode.\r\n"
                            , 40
                            );

                esp_set_mode( ESP_MODE_AP );
                NVIC_SystemReset();
            }
        }

        if ( ESP_RET_OK == e_ret )
        {
            /* Wait for device to become ready */
            sl_wait( 5, SL_TIME_SEC );

            /* Detect if temp sensor is present */
            if ( ds_no_of_dev != 0 )
            {
                bl_uart_send( UART_DBG
                            , (uint8_t*)"Live statistics:\r\n"
                            , 18
                            );
                while ( TRUE )
                {
                    /* Currently only one sensor is supported */
                    temp = ds18b20_read_temp( ds, 0 );
                    ds18b20_print_temp( out, temp );
                    e_ret = esp_send_temp( out );

                    if ( ESP_RET_OK == e_ret )
                    {
                        rly_state = esp_relay_get_state();

                        if ( ESP_RELAY_OFF != rly_state )
                        {
                            hal_msp_relay_set( GPIO_PIN_SET );
                        }
                        else
                        {
                            hal_msp_relay_set( GPIO_PIN_RESET );
                        }

                        e_ret = esp_send_ack();
                    }

                    if ( ESP_RET_OK != e_ret )
                    {
                        esp_log_error( e_ret );
                    }
                    else
                    {
                        esp_update_stats();
                    }
                    esp_dump_live_stats();
                    sl_wait( ESP_TEMP_SEND_REPEAT/3, SL_TIME_SEC );
                }
            }
            else
            {
                bl_uart_send( UART_DBG
                            , (uint8_t*)"DS18B20 not found.\r\n"
                            , 20
                            );
                bsp_blink_on_error( LED_ERROR );
            }
        }
    }

    while ( TRUE )
    {
        /* Should never reach this line */
    }

    return 0;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 24000000
  *            HCLK(Hz)                       = 24000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            HSE PREDIV1                    = 2
  *            PLLMUL                         = 6
  *            Flash Latency(WS)              = 0
  * @param  None
  * @retval None
  */
static void system_clk_cfg ( void )
{
    RCC_ClkInitTypeDef clkinitstruct = {0};
    RCC_OscInitTypeDef oscinitstruct = {0};

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
    oscinitstruct.HSEState        = RCC_HSE_ON;
    oscinitstruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV2;
    oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
    oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
    oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL6;
    if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
    {
        /* Initialization Error */
        while(1);
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    clkinitstruct.ClockType         = ( RCC_CLOCKTYPE_SYSCLK  \
                                      | RCC_CLOCKTYPE_HCLK    \
                                      | RCC_CLOCKTYPE_PCLK1   \
                                      | RCC_CLOCKTYPE_PCLK2   \
                                      );
    clkinitstruct.SYSCLKSource      = RCC_SYSCLKSOURCE_PLLCLK;
    clkinitstruct.AHBCLKDivider     = RCC_SYSCLK_DIV1;
    clkinitstruct.APB2CLKDivider    = RCC_HCLK_DIV1;
    clkinitstruct.APB1CLKDivider    = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clkinitstruct, LATENCY_0)!= HAL_OK)
    {
        /* Initialization Error */
        while(1);
    }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
