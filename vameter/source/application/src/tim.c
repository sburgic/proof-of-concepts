/**
 ** Name
 **   tim.c
 **
 ** Purpose
 **   Timer routines
 **
 ** Revision
 **   10-Oct-2020 (SSB) [] Initial
 **/

#include "tim.h"

static TIM_HandleTypeDef adc_tmr;
static TIM_HandleTypeDef ms_tmr;
static TIM_HandleTypeDef master_tmr;
static TIM_HandleTypeDef slave_tmr;

static volatile uint32_t system_timer_hi = 0;

static HAL_StatusTypeDef timebase_master_init( void )
{
    HAL_StatusTypeDef       ret;
    uint32_t                prescaler_val;
    TIM_MasterConfigTypeDef master_cfg;

    prescaler_val = (uint32_t)( SystemCoreClock / 1000000 ) - 1;

    master_tmr.Instance               = TIM3;
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

static HAL_StatusTypeDef timebase_slave_init( void )
{
    HAL_StatusTypeDef      ret = HAL_OK;
    TIM_SlaveConfigTypeDef slave_cfg;

    slave_tmr.Instance               = TIM2;
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
        HAL_NVIC_SetPriority( TIM2_IRQn, 0, 1 );
        HAL_NVIC_EnableIRQ( TIM2_IRQn );
        ret = HAL_TIM_Base_Start_IT( &slave_tmr );
    }

    return ret;
}

status_t tmr_bsp_init( void )
{
    status_t          ret = STATUS_OK;
    HAL_StatusTypeDef hret;

    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    hret  = timebase_master_init();
    hret |= timebase_slave_init();

    if ( HAL_OK != hret )
    {
        ret = STATUS_ERROR;
    }

    return ret;
}

status_t tmr_adc_init( void )
{
    status_t                       ret = STATUS_OK;
    HAL_StatusTypeDef              hret;
    TIM_MasterConfigTypeDef        master_cfg     = {0};
    TIM_OC_InitTypeDef             oc_cfg         = {0};
    TIM_BreakDeadTimeConfigTypeDef break_dead_cfg = {0};

    adc_tmr.Instance               = TIM1;
    adc_tmr.Init.Prescaler         = 99;
    adc_tmr.Init.CounterMode       = TIM_COUNTERMODE_UP;
    adc_tmr.Init.Period            = 99;
    adc_tmr.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    adc_tmr.Init.RepetitionCounter = 0;
    adc_tmr.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    master_cfg.MasterOutputTrigger = TIM_TRGO_OC1REF;
    master_cfg.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;

    oc_cfg.OCMode       = TIM_OCMODE_TOGGLE;
    oc_cfg.Pulse        = 0;
    oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
    oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
    oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
    oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    break_dead_cfg.OffStateRunMode  = TIM_OSSR_DISABLE;
    break_dead_cfg.OffStateIDLEMode = TIM_OSSI_DISABLE;
    break_dead_cfg.LockLevel        = TIM_LOCKLEVEL_OFF;
    break_dead_cfg.DeadTime         = 0;
    break_dead_cfg.BreakState       = TIM_BREAK_DISABLE;
    break_dead_cfg.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
    break_dead_cfg.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;

    __HAL_RCC_TIM1_CLK_ENABLE();

    hret  = HAL_TIM_OC_Init( &adc_tmr );
    hret |= HAL_TIMEx_MasterConfigSynchronization( &adc_tmr, &master_cfg );
    hret |= HAL_TIM_OC_ConfigChannel( &adc_tmr, &oc_cfg, TIM_CHANNEL_1 );
    hret |= HAL_TIMEx_ConfigBreakDeadTime( &adc_tmr, &break_dead_cfg );

    if ( HAL_OK != hret )
    {
        ret = STATUS_ERROR;
    }

    return ret;
}

status_t tmr_ms_init( void )
{
    HAL_StatusTypeDef       ret = HAL_OK;
    HAL_StatusTypeDef       hret;
    TIM_MasterConfigTypeDef master_cfg = {0};

    ms_tmr.Instance               = TIM7;
    ms_tmr.Init.Prescaler         = 239;
    ms_tmr.Init.Period            = 99;
    ms_tmr.Init.ClockDivision     = 0;
    ms_tmr.Init.CounterMode       = TIM_COUNTERMODE_UP;
    ms_tmr.Init.RepetitionCounter = 0;

    master_cfg.MasterOutputTrigger = TIM_TRGO_RESET;
    master_cfg.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;

    __HAL_RCC_TIM7_CLK_ENABLE();

    hret  = HAL_TIM_Base_Init( &ms_tmr );
    hret |= HAL_TIMEx_MasterConfigSynchronization( &ms_tmr, &master_cfg );

    if ( HAL_OK == ret )
    {
        HAL_NVIC_SetPriority( TIM7_IRQn, 1, 2 );
        HAL_NVIC_EnableIRQ( TIM7_IRQn );
        ret = HAL_TIM_Base_Start_IT( &ms_tmr );
    }

    return ret;
}

status_t tmr_adc_start( void )
{
    status_t          ret = STATUS_OK;
    HAL_StatusTypeDef hret;

    hret = HAL_TIM_OC_Start( &adc_tmr, TIM_CHANNEL_1 );

    if ( hret != HAL_OK )
    {
        ret = STATUS_ERROR;
    }

    return ret;
}

void get_time( Time_t* tv )
{
    volatile uint64_t hi_t;
    volatile uint64_t lo_t;
    uint16_t master_tmp;
    uint16_t slave_tmp;

    hi_t = (Time_t) system_timer_hi;

    slave_tmp   = slave_tmr.Instance->CNT;
    master_tmp  = master_tmr.Instance->CNT;

    if ( slave_tmp != slave_tmr.Instance->CNT )
    {
        slave_tmp  = slave_tmr.Instance->CNT;
        master_tmp = master_tmr.Instance->CNT;
    }

    lo_t = ((uint32_t) slave_tmp << 16 ) | ( master_tmp );

    *tv = ((Time_t) hi_t << 32 ) | (Time_t) lo_t;
}

void wait( Time_t time, Time_Base_t base )
{
    Time_t start_time = 0;
    Time_t act_time   = 0;
    Time_t delay      = 0;

    time = time * base;
    get_time( &start_time );
    do
    {
        get_time( &act_time );
        delay = act_time - start_time;
    } while ( delay < time );
}

void set_timeout( Time_t      time
                , Time_Base_t base
                , Time_t*     timeout
                )
{
    Time_t start_time;

    /* Assure usage of valid pointer only */
    if ( timeout != NULL )
    {
        start_time = (Time_t) 0;

        get_time( &start_time );
        *timeout = start_time + ( time * base );
    }
}

bool_t is_timeout( Time_t timeout )
{
    Time_t act_time;
    bool_t   ret;

    ret      = FALSE;
    act_time = (Time_t) 0;

    get_time( &act_time );

    if ( act_time >= timeout )
    {
        ret = TRUE;
    }

    return ret;
}

void bsp_tmr_slave_irq_hdl( void )
{
    HAL_TIM_IRQHandler( &slave_tmr );
}

void bsp_hi_sys_time_inc( void )
{
    system_timer_hi++;
}

void tmr_ms_irq_hdl( void )
{
    HAL_TIM_IRQHandler( &ms_tmr );
}

void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef* tmr )
{
    if ( TIM2 == tmr->Instance )
    {
        bsp_hi_sys_time_inc();
    }
    if ( TIM7 == tmr->Instance )
    {
        /* Do nothing here as another routine is called directly from IT */
    }
}
