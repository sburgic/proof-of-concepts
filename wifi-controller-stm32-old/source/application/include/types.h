/**
  ******************************************************************************
  * @file    application/include/types.h 
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.1
  * @date    25-Mar-2019
  * @brief   Custom types and definitions
  ******************************************************************************
 */

#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"
#include "stm32f1xx_hal_conf.h"

/* Used to check if pointer is valid */    
#ifndef NULL
    #define NULL ((void*)0)
#endif

/* Integer NULL */
#define ZERO (0)
 
/* Boolean results/values */
#define FALSE 0
#define TRUE  !(FALSE)

/*Boolean type */
typedef uint8_t bool_t;

typedef HAL_StatusTypeDef       HAL_Ret;
typedef GPIO_InitTypeDef        HAL_GPIO_Data;
typedef GPIO_PinState           HAL_GPIO_Pin_State;

typedef UART_InitTypeDef        UART_Peripheral_Cfg;
typedef USART_TypeDef*          UART_Base;

typedef TIM_HandleTypeDef       TMR_Peripheral;
typedef TIM_MasterConfigTypeDef TMR_Master_Cfg;
typedef TIM_SlaveConfigTypeDef  TMR_Slave_Cfg;

typedef FLASH_EraseInitTypeDef  Flash_Erase_Data;

#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */
