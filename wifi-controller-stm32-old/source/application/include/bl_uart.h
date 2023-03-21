/**
  ******************************************************************************
  * @file    application/include/bl_uart.h
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.1
  * @date    07-Jul-2018
  * @brief   UART module definitions
  ******************************************************************************
  */

#ifndef UART_H
#define UART_H

#include "stm32f1xx_hal_msp.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Define UART baudrate */
#define UART1_SPEED (115200)
#define UART2_SPEED UART1_SPEED

/* Select debug channel */
#define UART_DBG        UART2
#define UART_DBG_SPEED  UART2_SPEED

/* Select UART peripherals */
#define USE_BL_UART_1
#define USE_BL_UART_2

/* Generic UART buffer size */
#ifndef BL_UART_BUFF_SIZE
#define BL_UART_BUFF_SIZE (2048)
#endif

/* UART string delimiter */
#define BL_UART_STRING_DELIMITER '\n'

#ifndef BL_UART_1_BUFF_SIZE
#define BL_UART_1_BUFF_SIZE  BL_UART_BUFF_SIZE
#endif

#ifndef BL_UART_2_BUFF_SIZE
#define BL_UART_2_BUFF_SIZE  (32)
#endif

typedef struct
{
    uint8_t* buff;
    uint16_t size;
    uint16_t num;
    uint16_t in;
    uint16_t out;
    bool_t   initialized;
    uint8_t  str_delimiter;
    bool_t   dma;
} Bl_Uart_t;

HAL_Ret bl_uart_init ( UART_Base base, uint32_t baudrate );
HAL_Ret bl_uart_deinit ( UART_Base base );
uint8_t bl_uart_getc ( UART_Base base );
HAL_Ret bl_uart_receive( UART_Base base
                       , uint8_t*  buff
                       , uint16_t  size
                       , uint16_t* read_bytes
                       );
HAL_Ret bl_uart_send ( UART_Base base, uint8_t* data, uint16_t size );
bool_t bl_uart_buff_empty ( UART_Base base );
bool_t bl_uart_buff_full ( UART_Base base );
void bl_uart_buff_clear ( UART_Base base );
void bl_uart_set_custom_str_end_char ( UART_Base base, uint8_t c );
void bl_uart_insert_to_buff ( Bl_Uart_t* u, uint8_t c );
Bl_Uart_t* bl_uart_get_handle ( UART_Base base );
void bl_uart1_irq_hdl ( void );
void bl_uart2_irq_hdl ( void );

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
