/**
  ******************************************************************************
  * @file    application/include/uart.h
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.1
  * @date    01-Jun-2017
  * @brief   UART module definitions
  ******************************************************************************
  */

#ifndef UART_H
#define UART_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_DBG        UART2
#define UART_DBG_SPEED  UART2_SPEED

/* UART Timeout period used in polling mode only */
#define UART1_TIMEOUT    (uint32_t)(5000)
#define UART2_TIMEOUT    (uint32_t)(5000)
/* UART speed in bps */
#define UART1_SPEED      (uint32_t)(19200)
#define UART2_SPEED      (uint32_t)(9600)


/* Initialize UART peripheral */
HAL_Ret uart_init ( UART_Base       base
                  , uint32_t        speed
                  , UART_Work_Mode  mode
                  );

/* Send data over UART */
HAL_Ret uart_send_data ( UART_Base  base
                       , uint8_t*   data
                       , uint16_t   length
                       );

/* Receive data over UART */
HAL_Ret uart_receive_data ( UART_Base  base
                          , uint8_t*   data
                          , uint16_t   length
                          );

/* Store current UART mode in data buffer */
void uart_print_current_mode ( UART_Base base, uint8_t* data );

/* UART1 irq handler */
void uart_irq_hdl_main ( void );

/* Return pointer to UART instance */
UART_Peripheral* uart_get_handle ( UART_Base base );

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
