/**
  ******************************************************************************
  * @file    application/src/bsp_time.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    03-Jul-2017
  * @brief   BSP Time unit
  ******************************************************************************
 */

#include "bsp_time.h"
#include "stm32f1xx_hal_msp.h"
#include "sl_time.h"
#include "bsp_wifi_controller.h"

extern uint32_t SystemCoreClock;

/* BSP master timer peripheral handler */
static TMR_Peripheral       master_tmr;
/* BSP slave timer peripheral handler */
static TMR_Peripheral       slave_tmr;
/* 1ms tick timer */
static TMR_Peripheral       ms_tmr;
/* High 32-bits of 64-bits system time variable */
static volatile uint32_t    system_timer_hi = 0;

/* Configure and initialize BSP master timer */
static HAL_Ret bsp_tmr_master_init ( void );
/* Configure and initialize BSP slave timer */
static HAL_Ret bsp_tmr_slave_init ( void );


HAL_Ret bsp_tmr_init ( void )
{
    HAL_Ret ret;

    hal_msp_bsp_tmr_init ();

    ret  = bsp_tmr_master_init ();
    ret |= bsp_tmr_slave_init ();

    return ret;
}


HAL_Ret ms_tmr_init ( void )
{
    HAL_Ret     ret;
    uint32_t    prescaler_val;

    hal_msp_ms_tmr_init ();

    prescaler_val = (uint32_t)( SystemCoreClock / 1000000 ) - 1;

    ms_tmr.Instance                 = TMR_MS;
    ms_tmr.Init.Period              = 1000 - 1;
    ms_tmr.Init.Prescaler           = prescaler_val;
    ms_tmr.Init.ClockDivision       = 0;
    ms_tmr.Init.CounterMode         = TIM_COUNTERMODE_UP;
    ms_tmr.Init.RepetitionCounter   = 0;

    ret  = HAL_TIM_Base_Init ( &ms_tmr );
    if ( HAL_OK != ret )
    {
        bsp_blink_on_error ( LED_ERROR );
    }
    else
    {
        HAL_NVIC_SetPriority ( TMR_MS_IRQn, 0, 0 );
        HAL_NVIC_EnableIRQ ( TMR_MS_IRQn );
        ret = HAL_TIM_Base_Start_IT ( &ms_tmr );
    }

    return ret;
}


void bsp_get_time ( Sl_Time* tv )
{
    volatile uint64_t hi_t;
    volatile uint64_t lo_t;
    uint16_t master_tmp;
    uint16_t slave_tmp;

    hi_t = (Sl_Time)system_timer_hi;

    slave_tmp   = slave_tmr.Instance->CNT;
    master_tmp  = master_tmr.Instance->CNT;

    if ( slave_tmp != slave_tmr.Instance->CNT )
    {
        slave_tmp = slave_tmr.Instance->CNT;
        master_tmp  = master_tmr.Instance->CNT;
    }

    lo_t = ((uint32_t)slave_tmp << 16) | ( master_tmp );

    *tv = ((Sl_Time) hi_t << 32 ) | (Sl_Time) lo_t;
}


void tmr_master_irq_hdl ( void )
{
    HAL_TIM_IRQHandler ( &master_tmr );
}


void tmr_slave_irq_hdl ( void )
{
    HAL_TIM_IRQHandler ( &slave_tmr );
}


void tmr_ms_irq_hdl ( void )
{
    HAL_TIM_IRQHandler ( &ms_tmr );
}


void bsp_hi_sys_time_inc ( void )
{
    system_timer_hi++;
}


static HAL_Ret bsp_tmr_master_init ( void )
{
    HAL_Ret         ret;
    uint32_t        prescaler_val;
    TMR_Master_Cfg  master_cfg;

    prescaler_val = (uint32_t)( SystemCoreClock / 1000000 ) - 1;

    master_tmr.Instance                 = TMR_MASTER;
    master_tmr.Init.Period              = 0xFFFF;
    master_tmr.Init.Prescaler           = prescaler_val;
    master_tmr.Init.ClockDivision       = 0;
    master_tmr.Init.CounterMode         = TIM_COUNTERMODE_UP;
    master_tmr.Init.RepetitionCounter   = 0;

    master_cfg.MasterOutputTrigger      = TIM_TRGO_UPDATE;
    master_cfg.MasterSlaveMode          = TIM_MASTERSLAVEMODE_ENABLE;

    ret  = HAL_TIM_Base_Init ( &master_tmr );
    ret |= HAL_TIMEx_MasterConfigSynchronization ( &master_tmr, &master_cfg );
    if ( HAL_OK != ret )
    {
        bsp_blink_on_error ( LED_ERROR );
    }
    else
    {
        //HAL_NVIC_SetPriority ( TMR_MASTER_IQRn, 0, 0 );
        //HAL_NVIC_EnableIRQ ( TMR_MASTER_IQRn );
        ret = HAL_TIM_Base_Start ( &master_tmr );
    }

    return ret;
}


static HAL_Ret bsp_tmr_slave_init ( void )
{
    HAL_Ret         ret = HAL_OK;
    TMR_Slave_Cfg   slave_cfg;

    slave_tmr.Instance                  = TMR_SLAVE;
    slave_tmr.Init.Period               = 0xFFFF;
    slave_tmr.Init.ClockDivision        = 0;
    slave_tmr.Init.CounterMode          = TIM_COUNTERMODE_UP;
    slave_tmr.Init.RepetitionCounter    = 0;

    slave_cfg.SlaveMode                 = TIM_SLAVEMODE_EXTERNAL1;
    slave_cfg.InputTrigger              = TIM_TS_ITR2;

    ret  = HAL_TIM_Base_Init ( &slave_tmr );
    ret |= HAL_TIM_SlaveConfigSynchronization ( &slave_tmr, &slave_cfg );

    if ( HAL_OK != ret )
    {
        bsp_blink_on_error ( LED_ERROR );
    }
    else
    {
        HAL_NVIC_SetPriority ( TMR_SLAVE_IRQn, 0, 1 );
        HAL_NVIC_EnableIRQ ( TMR_SLAVE_IRQn );
        ret = HAL_TIM_Base_Start_IT ( &slave_tmr );
    }

    return ret;
}
