/**
 ** Name
 **   adc.c
 **
 ** Purpose
 **   ADC routines
 **
 ** Revision
 **   10-Oct-2020 (SSB) [] Initial
 **/

#include "adc.h"

#include "tim.h"

#include <math.h>

static ADC_HandleTypeDef adc_hdl;
static DMA_HandleTypeDef hdma_adc1;

static bool_t   adc_rms_flag = FALSE;
static uint16_t dma_data[ADC_DMA_BUFF_SIZE] = {0};

static uint16_t* adc_get_dma_buff_ready( void )
{
    static bool_t change = FALSE;
    uint16_t*     ret;

    if ( FALSE != change )
    {
        ret = &dma_data[ADC_DMA_BUFF_SIZE/2];
    }
    else
    {
        ret = &dma_data[0];
    }

    change = ~change;

    HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_15 );

    return ret;
}

status_t adc_init( void )
{
    status_t               ret = STATUS_OK;
    HAL_StatusTypeDef      hret;
    ADC_ChannelConfTypeDef adc_ch_cfg = {0};

    adc_hdl.Instance                   = ADC1;
    adc_hdl.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    adc_hdl.Init.ContinuousConvMode    = DISABLE;
    adc_hdl.Init.DiscontinuousConvMode = DISABLE;
    adc_hdl.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
    adc_hdl.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    adc_hdl.Init.NbrOfConversion       = 2;

    __HAL_RCC_DMA1_CLK_ENABLE();
    HAL_NVIC_SetPriority( DMA1_Channel1_IRQn, 2, 3 );
    HAL_NVIC_EnableIRQ( DMA1_Channel1_IRQn );

    hret = HAL_ADC_Init( &adc_hdl );

    adc_ch_cfg.Channel      = ADC_CHANNEL_0;
    adc_ch_cfg.Rank         = ADC_REGULAR_RANK_1;
    adc_ch_cfg.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;

    hret |= HAL_ADC_ConfigChannel( &adc_hdl, &adc_ch_cfg );

    adc_ch_cfg.Channel = ADC_CHANNEL_2;
    adc_ch_cfg.Rank    = ADC_REGULAR_RANK_2;

    hret |= HAL_ADC_ConfigChannel( &adc_hdl, &adc_ch_cfg );

    if ( HAL_OK != hret )
    {
        ret = STATUS_ERROR;
    }
    else
    {
        ret = tmr_adc_init();
    }

    return ret;
}

status_t adc_start( void )
{
    status_t          ret = STATUS_ERROR;
    HAL_StatusTypeDef hret;

    hret = HAL_ADCEx_Calibration_Start( &adc_hdl );

    if ( HAL_OK == hret )
    {
        wait( 10, TIME_MSEC );
        hret = HAL_ADC_Start_DMA( &adc_hdl
                                , (uint32_t*) dma_data
                                , ADC_DMA_BUFF_SIZE
                                );
    }

    if ( HAL_OK == hret )
    {
        ret = tmr_adc_start();
    }

    return ret;
}

status_t adc_stop( void )
{
    status_t          ret = STATUS_OK;
    HAL_StatusTypeDef hret;

    hret = HAL_ADC_Stop_DMA( &adc_hdl );

    if ( HAL_OK != hret )
    {
        ret = STATUS_ERROR;
    }

    return ret;
}

void adc_calc_rms( Adc_Rms_t* rms )
{
    uint16_t*       frame_r;
    uint16_t        i;
    static uint16_t ch_idx = 0;

    frame_r = adc_get_dma_buff_ready();

    for ( i = 0; i < ADC_DMA_BUFF_SIZE / 2; i++ )
    {
        if ( rms[ch_idx].curr_cnt < rms[ch_idx].req_samples )
        {
            uint32_t value;

            if ( 0 == ch_idx )
            {
                /* For the current sensing with ACS71240 zero is a Vref/2 */
                if ( frame_r[i] >= 2048 )
                {
                    value = frame_r[i] - 2048;
                }
                else
                {
                    value = 2048 - frame_r[i];
                }
            }
            else
            {
                value = frame_r[i];
            }

            rms[ch_idx].sum += ( value * value );
            rms[ch_idx].curr_cnt++;
        }
        else
        {
            /* Do the math if enough number of samples is collected
             * for the RMS evaluation
             */
            uint32_t tmp;

            tmp = rms[ch_idx].sum
                / rms[ch_idx].req_samples;

            rms[ch_idx].last = sqrt( tmp );

            rms[ch_idx].curr_cnt = 0;
            rms[ch_idx].sum      = 0;
        }

        ch_idx++;
        ch_idx = ch_idx % 2;
    }
}

void adc_set_rms_flag( bool_t state )
{
    adc_rms_flag = state;
}

bool_t adc_get_rms_flag( void )
{
    return adc_rms_flag;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adc )
{
    GPIO_InitTypeDef  GPIO_InitStruct = {0};

    if ( ADC1 == adc->Instance )
    {
        __HAL_RCC_ADC1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* ADC1 GPIO Configuration
        * PA0-WKUP ------> ADC1_IN0
        * PA2      ------> ADC1_IN2
        */
        GPIO_InitStruct.Pin  = GPIO_PIN_0 | GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

        hdma_adc1.Instance                 = DMA1_Channel1;
        hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
        hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
        hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
        hdma_adc1.Init.Mode                = DMA_CIRCULAR;
        hdma_adc1.Init.Priority            = DMA_PRIORITY_LOW;

        HAL_DMA_Init( &hdma_adc1 );
        __HAL_LINKDMA( adc, DMA_Handle, hdma_adc1 );

    }
}

void dma1_ch1_irq_hdl( void )
{
    HAL_DMA_IRQHandler( &hdma_adc1 );

    /* This handler is invoked twice - at half-full and full buffer */
    adc_set_rms_flag( TRUE );
}
