/**
 ** Name
 **   tim.h
 **
 ** Purpose
 **   Timer routines
 **
 ** Revision
 **   20-Apr-2020 (SSB) [] Initial
 **/

#ifndef __TIM_H__
#define __TIM_H__

#include "ptypes.h"

#include <stm32f1xx_hal.h>

typedef enum
{
    TIME_USEC = 1,
    TIME_MSEC = 1000,
    TIME_SEC  = 1000000
} Time_Base_t;

typedef uint64_t Time_t;

status_t tmr_bsp_init( void );
status_t tmr_ms_init( void );
status_t tmr_adc_init( void );
status_t tmr_adc_start( void );
void get_time( Time_t* tv );
void wait( Time_t time, Time_Base_t base );
bool_t is_timeout( Time_t timeout );
void set_timeout( Time_t      time
                , Time_Base_t base
                , Time_t*     timeout
                );
void bsp_tmr_slave_irq_hdl( void );
void bsp_hi_sys_time_inc( void );
void tmr_ms_irq_hdl( void );

#endif /* __TIM_H__ */
