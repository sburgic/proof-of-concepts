/**
  ******************************************************************************
  * @file    application/include/stm32f1xx_hal_msp.h
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    29-Jun-2017
  * @brief   HAL MSP module definitions.
  ******************************************************************************
 */

#ifndef STM32F1XX_HAL_MSP_H
#define STM32F1XX_HAL_MSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "stm32f100xb.h"

/* BSP Timer defines */
#define TMR_MASTER                      TIM3
#define TMR_MASTER_CLK_ENALBE()         __HAL_RCC_TIM3_CLK_ENABLE()
#define TMR_MASTER_IQRn                TIM3_IRQn

#define TMR_SLAVE                       TIM2
#define TMR_SLAVE_CLK_ENABLE()          __HAL_RCC_TIM2_CLK_ENABLE()
#define TMR_SLAVE_IRQn                  TIM2_IRQn

/* Coin impulse duration timer defines */
#define TMR_MS                          TIM4
#define TMR_MS_CLK_ENABLE()             __HAL_RCC_TIM4_CLK_ENABLE()
#define TMR_MS_IRQn                     TIM4_IRQn    

/* UART defines */
#define UART1                           USART1
#define UART1_TX_PIN                    GPIO_PIN_9
#define UART1_RX_PIN                    GPIO_PIN_10
#define UART1_TX_GPIO_PORT              GPIOA
#define UART1_RX_GPIO_PORT              GPIOA

#define UART2                           USART2
#define UART2_TX_PIN                    GPIO_PIN_2
#define UART2_RX_PIN                    GPIO_PIN_3
#define UART2_TX_GPIO_PORT              GPIOA
#define UART2_RX_GPIO_PORT              GPIOA

/* Dallas 18B20 temperature sensor defines */
#define DS18B20_PIN                     GPIO_PIN_8
#define DS18B20_GPIO_PORT               GPIOA

/* Relay */
#define RELAY_PIN                       GPIO_PIN_5
#define RELAY_GPIO_PORT                 GPIOB

/* Initialize UART low level resources */
HAL_Ret hal_msp_uart_init ( UART_Base base );
/* Initialize BSP Timer low level resources */
void hal_msp_bsp_tmr_init ( void );
/* Initialize 1ms Timer low level resources */
void hal_msp_ms_tmr_init ( void );
/* Initialize relay output */
void hal_msp_relay_init ( void );
/* Turn the relay on/off */
void hal_msp_relay_set ( GPIO_PinState new_state );

#ifdef __cplusplus
}
#endif

#endif /* STM32F1XX_HAL_MSP_H */
