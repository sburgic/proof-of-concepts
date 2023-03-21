/**
  ******************************************************************************
  * @file    bsp/bsp_wifi_controller/bsp_wifi_controller.h
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    23-May-2017
  * @brief   This file contains definitions for WIFI CONTROLLER board 
  *          Leds and push-buttons.
  ******************************************************************************
  */
 
#ifndef __WIFI_CONTROLLER_H
#define __WIFI_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* On-board LED definitions */
typedef enum 
{
    LED_STATUS = 0,
    LED_ERROR  = LED_STATUS
} BSP_LED;

/* Number of LEDs present on board */
#define NUM_OF_LEDS                      (1)

/* Number of buttons present on board */
#define NUM_OF_BUTTONS                   (1)

/* On-board button definitions */
typedef enum 
{
  S2_BUTTON = 0,
} BSP_Button;

/* On-board button modes */
typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} BSP_Button_Mode;

/**
 * @brief On board LEDs definitions
 */
#define LED_STATUS_PIN                  GPIO_PIN_11
#define LED_STATUS_GPIO_PORT            GPIOA
#define LED_STATUS_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()
#define LED_STATUS_GPIO_CLK_DISABLE()   __HAL_RCC_GPIOA_CLK_DISABLE()

#define LEDx_GPIO_CLK_ENABLE(__INDEX__)                                        \
                    do { if((__INDEX__) == 0) LED_STATUS_GPIO_CLK_ENABLE();    \
                       } while(0)

#define LEDx_GPIO_CLK_DISABLE(__INDEX__)                                       \
                    do { if((__INDEX__) == 0) LED_STATUS_GPIO_CLK_DISABLE();   \
                       } while(0)

#define S2_BUTTON_PIN                  GPIO_PIN_10
#define S2_BUTTON_GPIO_PORT            GPIOB
#define S2_BUTTON_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define S2_BUTTON_GPIO_CLK_DISABLE()   __HAL_RCC_GPIOB_CLK_DISABLE()
#define S2_BUTTON_EXTI_IRQn            EXTI15_10_IRQn

/**
  * @brief  Returns BSP version.
  * @param  None
  * @retval uint32_t BSP version
  */
uint32_t  bsp_get_version ( void );

/**
 * @brief  Store current BSP version into buffer.
 * @param  buff
 * @return uint16_t buffer length
 */
uint16_t bsp_print_version ( uint8_t* buff );

/**
  * @brief  Configures LED GPIO.
  * @param  Led: Specifies the Led to be configured. 
  *   This parameter can be one of following parameters:
  *     @arg LED_STATUS
  *     @arg LED_ERROR
  * @retval None
  */
void bsp_led_init ( BSP_LED led );

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on. 
  *   This parameter can be one of following parameters:
  *     @arg LED_STATUS
  *     @arg LED_ERROR
  * @retval None
  */
void bsp_led_on ( BSP_LED led );

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off. 
  *   This parameter can be one of following parameters:
  *     @arg LED_STATUS
  *     @arg LED_ERROR
  * @retval None
  */
void bsp_led_off ( BSP_LED led );

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled. 
  *   This parameter can be one of following parameters:
  *     @arg LED_STATUS
  *     @arg LED_ERROR
  * @retval None
  */
void bsp_led_toggle ( BSP_LED led );
 
/**
 * @brief  Blink led in infinite loop to signalize fatal error.
 * @param  led
 * @retval void
 */
void bsp_blink_on_error ( BSP_LED led );

/* FIXME: Add description */
void bsp_blink_alive ( void );

/**
  * @brief  Configures Button GPIO and EXTI Line.
  * @param  Button: Specifies the Button to be configured.
  *   This parameter should be: BUTTON_USER
  * @param  Button_Mode: Specifies Button mode.
  *   This parameter can be one of following parameters:   
  *     @arg BUTTON_MODE_GPIO: Button will be used as simple IO 
  *     @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line with
  *                            interrupt generation capability  
  * @retval None
  */
void bsp_pb_init ( BSP_Button button, BSP_Button_Mode mode );

/**
  * @brief  Returns the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *   This parameter should be: BUTTON_USER  
  * @retval Button state.
  */
uint32_t bsp_pb_get_state ( BSP_Button button );

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_CONTROLLER_H */
