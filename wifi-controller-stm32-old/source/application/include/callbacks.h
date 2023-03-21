/**
  ******************************************************************************
  * @file    application/include/callbacks.h 
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    20-Jul-2017
  * @brief   Header for re-implemented HAL callbacks
  ******************************************************************************
 */

#ifndef CALLBACKS_H
#define CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

void HAL_TIM_PeriodElapsedCallback ( TMR_Peripheral *tmr );

#ifdef __cplusplus
}
#endif

#endif /* CALLBACKS_H */
