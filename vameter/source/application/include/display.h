/**
 ** Name
 **   display.h
 **
 ** Purpose
 **   7-segment display routines
 **
 ** Revision
 **   10-Oct-2020 (SSB) [] Initial
 **/

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "ptypes.h"

#include <stm32f1xx_hal.h>

#define DISP1_SEG_A_PIN     GPIO_PIN_15
#define DISP1_SEG_A_PORT    GPIOC
#define DISP1_SEG_B_PIN     GPIO_PIN_14
#define DISP1_SEG_B_PORT    GPIOC
#define DISP1_SEG_C_PIN     GPIO_PIN_13
#define DISP1_SEG_C_PORT    GPIOC
#define DISP1_SEG_D_PIN     GPIO_PIN_9
#define DISP1_SEG_D_PORT    GPIOB
#define DISP1_SEG_E_PIN     GPIO_PIN_8
#define DISP1_SEG_E_PORT    GPIOB
#define DISP1_SEG_F_PIN     GPIO_PIN_7
#define DISP1_SEG_F_PORT    GPIOB
#define DISP1_SEG_G_PIN     GPIO_PIN_6
#define DISP1_SEG_G_PORT    GPIOB
#define DISP1_SEG_H_PIN     GPIO_PIN_5
#define DISP1_SEG_H_PORT    GPIOB
#define DISP1_T1_PIN        GPIO_PIN_4
#define DISP1_T1_PORT       GPIOB
#define DISP1_T2_PIN        GPIO_PIN_3
#define DISP1_T2_PORT       GPIOB
#define DISP1_T3_PIN        GPIO_PIN_15
#define DISP1_T3_PORT       GPIOA
#define DISP1_T4_PIN        GPIO_PIN_12
#define DISP1_T4_PORT       GPIOA
#define DISP2_SEG_A_PIN     GPIO_PIN_3
#define DISP2_SEG_A_PORT    GPIOA
#define DISP2_SEG_B_PIN     GPIO_PIN_4
#define DISP2_SEG_B_PORT    GPIOA
#define DISP2_SEG_C_PIN     GPIO_PIN_5
#define DISP2_SEG_C_PORT    GPIOA
#define DISP2_SEG_D_PIN     GPIO_PIN_6
#define DISP2_SEG_D_PORT    GPIOA
#define DISP2_SEG_E_PIN     GPIO_PIN_7
#define DISP2_SEG_E_PORT    GPIOA
#define DISP2_SEG_F_PIN     GPIO_PIN_0
#define DISP2_SEG_F_PORT    GPIOB
#define DISP2_SEG_G_PIN     GPIO_PIN_1
#define DISP2_SEG_G_PORT    GPIOB
#define DISP2_SEG_H_PIN     GPIO_PIN_2
#define DISP2_SEG_H_PORT    GPIOB
#define DISP2_T1_PIN        GPIO_PIN_10
#define DISP2_T1_PORT       GPIOB
#define DISP2_T2_PIN        GPIO_PIN_11
#define DISP2_T2_PORT       GPIOB
#define DISP2_T3_PIN        GPIO_PIN_8
#define DISP2_T3_PORT       GPIOA
#define DISP2_T4_PIN        GPIO_PIN_11
#define DISP2_T4_PORT       GPIOA

#define DISP1_CLOCK_ENABLE() do { __HAL_RCC_GPIOA_CLK_ENABLE(); \
                                  __HAL_RCC_GPIOB_CLK_ENABLE(); \
                                  __HAL_RCC_GPIOC_CLK_ENABLE(); } while(0)

#define DISP2_CLOCK_ENABLE() do { __HAL_RCC_GPIOA_CLK_ENABLE(); \
                                  __HAL_RCC_GPIOB_CLK_ENABLE(); } while(0)

#endif /* __DISPLAY_H__ */
