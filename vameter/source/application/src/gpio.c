/**
 ** Name
 **   gpio.c
 **
 ** Purpose
 **   GPIO routines
 **
 ** Revision
 **   10-Oct-2020 (SSB) [] Initial
 **/

#include "gpio.h"

status_t gpio_init( void )
{
    status_t         ret             = STATUS_OK;
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    CLI_PB_CLOCK_ENABLE();
    JUMPER_CLOCK_ENABLE();
    ACS_FAULT_CLOCK_ENABLE();

    HAL_GPIO_WritePin( CLI_PB_PORT
                     , CLI_PB_PIN
                     , GPIO_PIN_SET
                     );
    HAL_GPIO_WritePin( JUMPER_PORT
                     , JUMPER_PIN
                     , GPIO_PIN_SET
                     );
    HAL_GPIO_WritePin( ACS_FAULT_PORT
                     , ACS_FAULT_PIN
                     , GPIO_PIN_SET
                     );

    GPIO_InitStruct.Pin   = CLI_PB_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( CLI_PB_PORT, &GPIO_InitStruct );

    GPIO_InitStruct.Pin   = JUMPER_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( JUMPER_PORT, &GPIO_InitStruct );

    GPIO_InitStruct.Pin   = ACS_FAULT_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( ACS_FAULT_PORT, &GPIO_InitStruct );

    return ret;
}
