/**
  ******************************************************************************
  * @file    application/src/callbacks.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    20-Jul-2017
  * @brief   Re-implemented HAL callbacks
  ******************************************************************************
 */

#include "callbacks.h"
#include "stm32f1xx_hal_msp.h"
#include "bsp_time.h"

void HAL_TIM_PeriodElapsedCallback ( TMR_Peripheral *tmr )
{
    if ( TMR_MASTER == tmr->Instance )
    {
        // Nothing to do here..
    }

    if ( TMR_SLAVE == tmr->Instance )
    {
        bsp_hi_sys_time_inc ();
    }
}
