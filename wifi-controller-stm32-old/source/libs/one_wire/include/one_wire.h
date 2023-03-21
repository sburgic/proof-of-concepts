/**
  ******************************************************************************
  * @file    hal/one_wire/include/one_wire.h
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.1
  * @date    25-Mar-2019
  * @brief   1-Wire definitions
  ******************************************************************************
 */

#ifndef ONE_WIRE_H
#define ONE_WIRE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stm32f1xx_hal.h"
#include "types.h"
#include "stm32f1xx_hal_msp.h"
#include "bl_uart.h"

/* Attached UART base */
#define OW_UART_BASE    UART_DBG
    
#define OW_ROM_SIZE     (8)             /**< 1-Wire device ROM size in bytes */
#define OW_MAX_DEVICES  (10)            /**< Maximum number of devices 
                                         *   on 1-Wire bus
                                         */
#define OW_NO_DEVICES   ((uint8_t)0)    /**< No devices attached to
                                         *   1-Wire bus
                                         */
    
/* 1-Wire basic commands */
#define OW_CMD_SEARCH_ROM       (0xF0)      /**< Multiple devices on the bus */
#define OW_CMD_READ_ROM         (0x33)      /**< Single device on the bus    */
#define OW_CMD_SKIP_ROM         (0xCC)
#define OW_CMD_MATCH_ROM        (0x55)
#define OW_CMD_CONVERT_TEMP     (0x44)
#define OW_CMD_READ_SCHRATCHPAD (0xBE)

/* Main 1-Wire structure */
typedef struct
{
    GPIO_TypeDef*       port;               /**< GPIO port for 1-Wire bus */
    volatile uint32_t*  reg;                /**< GPIO register for 1-Wire bus */
    uint32_t            reg_mask;           /**< GPIO register mask */
    uint32_t            input_mask;         /**< GPIO input mask */
    uint32_t            output_mask;        /**< GPIO output mask */
    uint16_t            bitmask;            /**< GPIO pin mask */
    uint8_t             rom[8];             /**< 1-Wire ROM */
    uint8_t             last_discrepancy;
    uint8_t             last_family_discrepancy;
    bool_t              last_device_flag;
} One_Wire;


/* Initialize 1-Wire bus */
HAL_Ret ow_init ( One_Wire*     ow
                , GPIO_TypeDef* gpio_port
                , uint16_t      gpio_pin
                );
/* Send reset pulse to 1-Wire bus */
bool_t ow_reset ( One_Wire* ow );
/* Search for devices on 1-Wire bus */
bool_t ow_search ( One_Wire* ow, uint8_t* addr_buff );
/* Write byte to 1-Wire bus */
void ow_write_byte ( One_Wire* ow, uint8_t byte );
/* Read byte from 1-Wire bus */
uint8_t ow_read_byte ( One_Wire* ow );
/* Calculate CRC8 */
uint8_t ow_crc8 ( uint8_t* buff, uint8_t length );
/* List all devices on 1-Wire bus */
uint8_t ow_list_all_devices ( One_Wire* ow );
/* Match specific device using ROM */
bool_t ow_match_rom ( One_Wire*   ow 
                    , uint8_t     dev_idx
                    );
/* Issue convert temperature instruction to all devices on 1-Wire bus */
bool_t ow_convert_temp ( One_Wire* ow );


#ifdef __cplusplus
}
#endif

#endif /* ONE_WIRE_H */
