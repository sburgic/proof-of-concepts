/**
 ** Name
 **   interrupt.c
 **
 ** Purpose
 **   IRQ routines
 **
 ** Revision
 **   21-Apr-2021 (SSB) [] Initial
 **/

#include "interrupt.h"

#include "time.h"

extern volatile bool_t pb1;
extern volatile bool_t pb2;
extern bool_t          init_done;

void NMI_Handler( void )
{
}

void HardFault_Handler( void )
{
    while (1)
    {;}
}

void MemManage_Handler( void )
{
    while (1)
    {;}
}

void BusFault_Handler( void )
{
    while (1)
    {;}
}

void UsageFault_Handler( void )
{
    while (1)
    {;}
}

void DebugMon_Handler( void )
{
    while (1)
    {;}
}

void SysTick_Handler( void )
{
    HAL_IncTick();

    GPIO_PinState pin1;
    GPIO_PinState pin2;

    static volatile uint8_t debounce1 = 75;
    static volatile uint8_t debounce2 = 75;

    if ( FALSE != init_done )
    {
        pin1 = HAL_GPIO_ReadPin( GPIOB, GPIO_PIN_10 );
        pin2 = HAL_GPIO_ReadPin( GPIOA, GPIO_PIN_10 );

        if ( GPIO_PIN_RESET == pin1 )
        {
            debounce1--;

            if ( 0 == debounce1 )
            {
                pb1 = TRUE;
            }
        }
        else
        {
            debounce1 = 75;
        }

        if ( GPIO_PIN_RESET == pin2 )
        {
            debounce2--;

            if ( 0 == debounce2 )
            {
                pb2 = TRUE;
            }
        }
        else
        {
            debounce2 = 75;
        }
    }
}

void TIM2_IRQHandler( void )
{
    bsp_tmr_slave_irq_hdl();
}

void TIM4_IRQHandler( void )
{
    bsp_tmr_sched_irq_hdl();
}
