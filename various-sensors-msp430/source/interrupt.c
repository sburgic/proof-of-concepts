#include <msp430.h>

#include "system.h"

#pragma vector=TIMER2_A0_VECTOR
__interrupt void Timer2_A2( void )
{
    timer2_A2_irq_hdl();
}
