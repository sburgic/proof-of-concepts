/**
  ******************************************************************************
  * @file    application/include/stm32f1xx_it.h 
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    01-Jun-2017
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  */

#ifndef __STM32F1xx_IT_H
#define __STM32F1xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler ( void );
void TIM3_IRQHandler(void);
void TIM2_IRQHandler ( void );
void TIM4_IRQHandler ( void );
void EXTI2_IRQHandler ( void );

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1xx_IT_H */
