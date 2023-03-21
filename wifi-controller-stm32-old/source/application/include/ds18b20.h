/**
  ******************************************************************************
  * @file    application/include/ds18b20.h
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    17-Avg-2017
  * @brief   Dallas 18B20 temperature sensor high level driver defines
  ******************************************************************************
 */

#ifndef DS18B20_H
#define DS18B20_H

#ifdef __cplusplus
extern "C" {
#endif

#include "one_wire.h"
    
    
typedef One_Wire    DS_18B20;
typedef DS_18B20*   DS_18B20_Hdl;

/* Initialize DS18B20 */
HAL_Ret ds18b20_init ( DS_18B20_Hdl   ds_hdl
                     , GPIO_TypeDef*  gpio_port
                     , uint16_t       gpio_pin
                     );

/* Return DS18B20 handle */
DS_18B20* ds18b20_get_handle ( void );


/* Read temperature from DS18B20 */
int16_t ds18b20_read_temp ( DS_18B20_Hdl ds, uint8_t dev_idx );

/* Print measured temperature into buffer */
void ds18b20_print_temp ( uint8_t* buff, int16_t temperature ); 


#ifdef __cplusplus
}
#endif

#endif /* DS18B20_H */
