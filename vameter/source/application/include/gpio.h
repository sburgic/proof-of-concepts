/**
 ** Name
 **   gpio.h
 **
 ** Purpose
 **   GPIO routines
 **
 ** Revision
 **   10-Oct-2020 (SSB) [] Initial
 **/

#ifndef __GPIO_H__
#define __GPIO_H__

#include "ptypes.h"

#include <stm32f1xx_hal.h>

#define CLI_PB_PIN              GPIO_PIN_2
#define CLI_PB_PORT             GPIOA
#define CLI_PB_CLOCK_ENABLE()   do { __HAL_RCC_GPIOA_CLK_ENABLE(); } while(0)

#define JUMPER_PIN             GPIO_PIN_14
#define JUMPER_PORT            GPIOB
#define JUMPER_CLOCK_ENABLE()   do { __HAL_RCC_GPIOB_CLK_ENABLE(); } while(0)

#define ACS_FAULT_PIN            GPIO_PIN_15
#define ACS_FAULT_PORT           GPIOB
#define ACS_FAULT_CLOCK_ENABLE() do { __HAL_RCC_GPIOB_CLK_ENABLE(); } while(0)

status_t gpio_init( void );

#endif /* __GPIO_H__ */
