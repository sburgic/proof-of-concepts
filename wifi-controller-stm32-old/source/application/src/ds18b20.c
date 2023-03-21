/**
  ******************************************************************************
  * @file    application/src/ds18b20.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    17-Avg-2017
  * @brief   Dallas 18B20 temperature sensor high level driver
  ******************************************************************************
 */

#include "ds18b20.h"
#include "sl_handle.h"
#include "sl_string.h"
#include "types.h"

/* Sensor error - value that never appears in measurement */
#define DS_SENSOR_ERROR   ((int16_t)0xFF80)

/* DS handle */
static DS_18B20 ds_hdl;


HAL_Ret ds18b20_init ( DS_18B20_Hdl     ds
                     , GPIO_TypeDef*    gpio_port
                     , uint16_t         gpio_pin
                     )
{
    HAL_Ret ret = HAL_INV_HDL;

    if ( HDL_IS_VALID( ds ) )
    {
        ret = ow_init ( (One_Wire*)ds, gpio_port, gpio_pin );
    }

    return ret;
}


DS_18B20* ds18b20_get_handle ( void )
{
    return &ds_hdl;
}


int16_t ds18b20_read_temp ( DS_18B20_Hdl ds, uint8_t dev_idx )
{
    int32_t temperature = DS_SENSOR_ERROR;
    bool_t  b_ret;

    b_ret = ow_reset ( ds );

    if ( FALSE != b_ret )
    {
        b_ret = ow_convert_temp ( ds );

        if ( FALSE != b_ret )
        {
            ow_match_rom ( ds, dev_idx );
            ow_write_byte ( ds, OW_CMD_READ_SCHRATCHPAD );

            temperature = ow_read_byte ( ds );
            temperature = ( ow_read_byte ( ds ) << 8 ) | temperature;
        }
    }

    return temperature;
}


void ds18b20_print_temp ( uint8_t* buff, int16_t temperature )
{
    int16_t fraction;

    if ( DS_SENSOR_ERROR != temperature )
    {
        sl_sprintf_d ( buff
                     , (uint8_t*)"%d.%d"
                     , (int32_t)((int16_t)( temperature >> 4 ))
                     , SL_MAX_STRING_SIZE
                     );

        fraction  = temperature & 0x000F; /**< Last 4 bits are decimals */

        /* Use one decimal place */
        fraction *= 625;
        fraction /= 1000;

        sl_sprintf_d ( buff
                     , buff
                     , (int32_t)((int16_t)fraction )
                     , SL_MAX_STRING_SIZE
                     );
    }
}
