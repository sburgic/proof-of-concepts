#include "mq3.h"

void mq3_init( void )
{
    MQ3_PB_START_INIT();
    MQ3_PB_AUX_INIT();
    SYSCFG2 |= MQ3_AN_IN_PIN;

    ADCCTL0 &= ~ADCENC;       /* Disable ADC */
    ADCCTL0 |= ADCON;         /* Enable ADC core */
    ADCCTL1 |= ADCSSEL1;      /* Set SMCLK as ADC clock source */
    ADCCTL2 |= ADCRES1;       /* 10-bit resolution */
    ADCMCTL0 |= MQ3_AN_IN_CH; /* Selected analog channels */
    ADCCTL0 |= ADCENC;        /* Enable ADC */
}

uint16_t mq3_get_value( void )
{
    uint16_t value = MQ3_ERR_VALUE;

    ADCCTL0 |= ADCSC; /* SAMPCON = 1 */
    __delay_cycles( 10 );
    ADCCTL0 &= ~ADCSC; /* SAMPCON = 0 */

    /* TODO: Add a timeout in case of unsuccessful conversion */
    while ( 0 != ( ADCCTL0 & 0x01 )); /* Wait until conversion is completed */

    value = ADCMEM0;

    return value;
}
