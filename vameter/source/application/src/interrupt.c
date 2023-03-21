/**
 ** Name
 **   interrupt.c
 **
 ** Purpose
 **   IRQ routines
 **
 ** Revision
 **   10-Oct-2020 (SSB) [] Initial
 **/

#include "interrupt.h"

#include "adc.h"
#include "tim.h"

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
    HAL_SYSTICK_IRQHandler();
}

void TIM2_IRQHandler( void )
{
    bsp_tmr_slave_irq_hdl();
}

void TIM7_IRQHandler( void )
{
    tmr_ms_irq_hdl();
}

void DMA1_Channel1_IRQHandler( void )
{
    dma1_ch1_irq_hdl();
}
