/**
 ** Name
 **   adc.h
 **
 ** Purpose
 **   ADC routines
 **
 ** Revision
 **   10-Oct-2020 (SSB) [] Initial
 **/

#ifndef __ADC_H__
#define __ADC_H__

#include "ptypes.h"

#include <stm32f1xx_hal.h>

#define ADC_DMA_BUFF_SIZE (512)

typedef struct
{
    uint32_t last;          /* Last valid calculated RMS */
    uint32_t max;           /* Max measured RMS value - test purpose only */
    uint32_t min;           /* Min measured RMS value - test purpose only */
    uint32_t curr_cnt;      /* Current sample counter */
    uint32_t req_samples;   /* No of samples required to evaluate RMS */
    uint64_t sum;           /* Current samples sum value */
} Adc_Rms_t;

status_t adc_init( void );
status_t adc_start( void );
status_t adc_stop( void );
void adc_calc_rms( Adc_Rms_t* rms );
void adc_set_rms_flag( bool_t state );
bool_t adc_get_rms_flag( void );
void dma1_ch1_irq_hdl( void );

#endif /* __ADC_H__ */
