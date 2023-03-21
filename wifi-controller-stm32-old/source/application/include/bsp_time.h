/**
  ******************************************************************************
  * @file    application/include/bsp_time.h 
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    03-Jul-2017
  * @brief   BSP Time unit
  ******************************************************************************
 */

#ifndef BSP_TIME_H
#define BSP_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "sl_time.h"

/* Initialize BSP Timer */
HAL_Ret bsp_tmr_init ( void );
/* Initialize 1ms Timer */
HAL_Ret ms_tmr_init ( void );
/* Get system time */
void bsp_get_time ( Sl_Time* tv );
/* BSP master timer irq handler */
void tmr_master_irq_hdl ( void );
/* BSP slave timer irq handler */
void tmr_slave_irq_hdl ( void );
/* Increment higher 32-bits of 64-bits system time variable */
void bsp_hi_sys_time_inc ( void );
/* 1ms timer irq handler */
void tmr_ms_irq_hdl ( void );

#ifdef __cplusplus
}
#endif

#endif /* BSP_TIME_H */
