/**
 ** Name
 **   time.c
 **
 ** Purpose
 **   Time routines
 **
 ** Revision
 **   21-Apr-2021 (SSB) [] Initial
 **/

#include "time.h"

static TIM_HandleTypeDef master_tmr;
static TIM_HandleTypeDef slave_tmr;
static volatile uint32_t system_timer_hi = 0;

static HAL_StatusTypeDef bsp_tmr_master_init( void )
{
    HAL_StatusTypeDef       ret;
    uint32_t                prescaler_val;
    TIM_MasterConfigTypeDef master_cfg;

    prescaler_val = (uint32_t)( SystemCoreClock / 1000000 ) - 1;

    master_tmr.Instance               = TMR_MASTER;
    master_tmr.Init.Period            = 0xFFFF;
    master_tmr.Init.Prescaler         = prescaler_val;
    master_tmr.Init.ClockDivision     = 0;
    master_tmr.Init.CounterMode       = TIM_COUNTERMODE_UP;
    master_tmr.Init.RepetitionCounter = 0;

    master_cfg.MasterOutputTrigger    = TIM_TRGO_UPDATE;
    master_cfg.MasterSlaveMode        = TIM_MASTERSLAVEMODE_ENABLE;

    ret  = HAL_TIM_Base_Init( &master_tmr );
    ret |= HAL_TIMEx_MasterConfigSynchronization( &master_tmr, &master_cfg );

    if ( HAL_OK == ret )
    {
        ret = HAL_TIM_Base_Start( &master_tmr );
    }

    return ret;
}

static HAL_StatusTypeDef bsp_tmr_slave_init( void )
{
    HAL_StatusTypeDef      ret = HAL_OK;
    TIM_SlaveConfigTypeDef slave_cfg;

    slave_tmr.Instance               = TMR_SLAVE;
    slave_tmr.Init.Period            = 0xFFFF;
    slave_tmr.Init.ClockDivision     = 0;
    slave_tmr.Init.CounterMode       = TIM_COUNTERMODE_UP;
    slave_tmr.Init.RepetitionCounter = 0;

    slave_cfg.SlaveMode              = TIM_SLAVEMODE_EXTERNAL1;
    slave_cfg.InputTrigger           = TIM_TS_ITR2;

    ret  = HAL_TIM_Base_Init( &slave_tmr );
    ret |= HAL_TIM_SlaveConfigSynchro( &slave_tmr, &slave_cfg );

    if ( HAL_OK == ret )
    {
        HAL_NVIC_SetPriority( TMR_SLAVE_IRQn, 0, 1 );
        HAL_NVIC_EnableIRQ( TMR_SLAVE_IRQn );
        ret = HAL_TIM_Base_Start_IT( &slave_tmr );
    }

    return ret;
}

status_t bsp_tmr_init( void )
{
    status_t          ret = STATUS_OK;
    HAL_StatusTypeDef hret;

    TMR_MASTER_CLK_ENABLE();
    TMR_SLAVE_CLK_ENABLE();

    hret  = bsp_tmr_master_init();
    hret |= bsp_tmr_slave_init();

    if ( HAL_OK != hret )
    {
        ret = STATUS_ERROR;
    }

    return ret;
}

void bsp_get_time( Bsp_Time* tv )
{
    volatile uint64_t hi_t;
    volatile uint64_t lo_t;
    uint16_t master_tmp;
    uint16_t slave_tmp;

    hi_t = (Bsp_Time) system_timer_hi;

    slave_tmp   = slave_tmr.Instance->CNT;
    master_tmp  = master_tmr.Instance->CNT;

    if ( slave_tmp != slave_tmr.Instance->CNT )
    {
        slave_tmp  = slave_tmr.Instance->CNT;
        master_tmp = master_tmr.Instance->CNT;
    }

    lo_t = ((uint32_t) slave_tmp << 16 ) | ( master_tmp );

    *tv  = ((Bsp_Time) hi_t << 32 );
    *tv |= (Bsp_Time) lo_t;
}

void bsp_tmr_slave_irq_hdl( void )
{
    HAL_TIM_IRQHandler( &slave_tmr );
}

void bsp_hi_sys_time_inc( void )
{
    system_timer_hi++;
}

void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef* tmr )
{
    if ( TMR_SLAVE == tmr->Instance )
    {
        bsp_hi_sys_time_inc();
    }
}

void bsp_wait( Bsp_Time time, Bsp_Time_Base base )
{
    Bsp_Time start_time = 0;
    Bsp_Time act_time   = 0;
    Bsp_Time delay      = 0;

    time = time * base;
    bsp_get_time( &start_time );
    do
    {
        bsp_get_time( &act_time );
        delay = act_time - start_time;
    } while ( delay < time );
}

void bsp_set_timeout( Bsp_Time      time
                    , Bsp_Time_Base base
                    , Bsp_Time*     timeout
                    )
{
    Bsp_Time start_time;

    /* Assure usage of valid pointer only */
    if ( timeout != NULL )
    {
        start_time = (Bsp_Time) 0;

        bsp_get_time( &start_time );
        *timeout = start_time + ( time * base );
    }
}

bool_t bsp_is_timeout( Bsp_Time timeout )
{
    Bsp_Time act_time;
    bool_t   ret;

    ret      = FALSE;
    act_time = (Bsp_Time) 0;

    bsp_get_time( &act_time );

    if ( act_time >= timeout )
    {
        ret = TRUE;
    }

    return ret;
}
