/**
  ******************************************************************************
  * @file    application/src/bl_uart.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    07-Jul-2018
  * @brief   UART module implementation
  ******************************************************************************
  */

#include "bl_uart.h"

#include <sl_handle.h>

/* Wait for TX empty */
#define UART_TX_EMPTY(UART) ((UART)->SR & USART_FLAG_TXE)
#define UART_WAIT(UART)     while(!UART_TX_EMPTY(UART))

#ifdef USE_BL_UART_1
    uint8_t uart_1_buffer[BL_UART_1_BUFF_SIZE];

    Bl_Uart_t BL_UART_1 =
    {
        .buff           = uart_1_buffer,
        .size           = BL_UART_1_BUFF_SIZE,
        .num            = 0,
        .in             = 0,
        .out            = 0,
        .initialized    = FALSE,
        .str_delimiter  = BL_UART_STRING_DELIMITER,
        .dma            = FALSE
    };
#endif

#ifdef USE_BL_UART_2
    uint8_t uart_2_buffer[BL_UART_2_BUFF_SIZE];

    Bl_Uart_t BL_UART_2 =
    {
        .buff           = uart_2_buffer,
        .size           = BL_UART_2_BUFF_SIZE,
        .num            = 0,
        .in             = 0,
        .out            = 0,
        .initialized    = FALSE,
        .str_delimiter  = BL_UART_STRING_DELIMITER,
        .dma            = FALSE
    };
#endif

#if !defined (USE_BL_UART_1) || !defined (USE_BL_UART_2)
    #error "DEFINE AT LEAST ONE UART"
#endif

static uint16_t bl_uart_find_char ( UART_Base base, uint8_t c )
{
    uint16_t   num;
    uint16_t   out;
    uint16_t   pos = 0;
    Bl_Uart_t* u;

    u = bl_uart_get_handle ( base );

    num = u->num;
    out = u->out;

    while ( num > 0 )
    {
        /* Check overflow */
        if ( out == u->size )
        {
            out = 0;
        }

        /* Check if characters matches */
        if (((uint8_t) u->buff[out] ) == ((uint8_t)c ))
        {
            pos = out;
            break;
        }

        out++;
        num--;
    }

    return pos;
}

HAL_Ret bl_uart_init ( UART_Base base, uint32_t baudrate )
{
    HAL_Ret ret = HAL_INV_HDL;
    UART_Peripheral huart;
    Bl_Uart_t*      u;

    if ( HDL_IS_VALID(base))
    {
        u   = bl_uart_get_handle ( base );
        ret = hal_msp_uart_init ( base );

        if ( HAL_OK == ret )
        {
            huart.Instance          = base;
            huart.Init.BaudRate     = baudrate;
            huart.Init.WordLength   = UART_WORDLENGTH_8B;
            huart.Init.StopBits     = UART_STOPBITS_1;
            huart.Init.Parity       = UART_PARITY_NONE;
            huart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
            huart.Init.Mode         = UART_MODE_TX_RX;

            ret = HAL_UART_Init ( &huart );

            if ( HAL_OK == ret )
            {
                if ( UART1 == base )
                {
                    HAL_NVIC_SetPriority ( USART1_IRQn, 2, 1 );
                    HAL_NVIC_EnableIRQ ( USART1_IRQn );

                }
                else
                {
                    HAL_NVIC_SetPriority ( USART2_IRQn, 3, 2 );
                    HAL_NVIC_EnableIRQ ( USART2_IRQn );
                }

                /* Enable Rx interrupt */
                base->CR1 |= USART_CR1_RXNEIE;

                /* Enable USART peripheral */
                base->CR1 |= USART_CR1_UE;

                u->initialized = TRUE;
            }
            else
            {
                ret = HAL_ERROR;
            }
        }
    }

    return ret;
}

HAL_Ret bl_uart_deinit( UART_Base base )
{
    HAL_Ret ret = HAL_INV_HDL;
    UART_Peripheral huart;
    Bl_Uart_t*      u;

    if ( HDL_IS_VALID( base ))
    {
        u = bl_uart_get_handle( base );
        huart.Instance = base;

        ret = HAL_UART_DeInit( &huart );

        if ( HAL_OK == ret )
        {
            u->initialized = FALSE;
        }
    }

    return ret;
}

uint8_t bl_uart_getc ( UART_Base base )
{
    uint8_t    c = 0;
    Bl_Uart_t* u;

    u = bl_uart_get_handle ( base );

    if (( u->num > 0) || ( u->in != u->out ))
    {
        /* Check overflow */
        if ( u->out == u->size  )
        {
            u->out = 0;
        }

        /* Read character */
        c = u->buff[u->out];

        /* Increase output pointer */
        u->out++;

        /* Decrease number of elements */
        if ( 0 != u->num )
        {
            u->num--;
        }
    }

    return c;
}

HAL_Ret bl_uart_receive( UART_Base base
                       , uint8_t*  buff
                       , uint16_t  size
                       , uint16_t* read_bytes
                       )
{
    HAL_Ret    ret = HAL_INV_HDL;
    int16_t    cnt = 0;
    uint16_t   pos = 0;
    Bl_Uart_t* u   = NULL;

    u = bl_uart_get_handle( base );

    if ( NULL != buff )
    {
        /* Check for any data on UART */
        pos = bl_uart_find_char( base, u->str_delimiter );

        if (( 0 == u->num ) || (( 0 == pos ) && ( u->num != u->size )))
        {
            ret = HAL_ERROR;
        }
        else
        {
            ret = HAL_OK;
        }
    }

    if ( HAL_OK == ret )
    {
        while ( cnt < ( size - 1 ))
        {
            buff[cnt] = bl_uart_getc( base );

            if ( buff[cnt] == u->str_delimiter )
            {
                break;
            }

            cnt++;
        }

        /* Update the number of read bytes */
        cnt++;

        if ( NULL != read_bytes )
        {   /* Update only if requested.
             * NULL as argument means read_bytes is not requested.
             */
            *read_bytes = cnt;
        }

        /*Add zero to the end of string */
        buff[cnt] = 0;
    }

    return ret;
}

HAL_Ret bl_uart_send ( UART_Base base, uint8_t* data, uint16_t size )
{
    HAL_Ret    ret = HAL_ERROR;
    uint16_t   i;
    Bl_Uart_t* u;

    u = bl_uart_get_handle ( base );

    if ( FALSE != u->initialized )
    {
        for ( i = 0; i < size; i++ )
        {
            /* Wait to be ready, buffer empty */
            UART_WAIT( base );
            /* Send actual data */
            base->DR = (uint16_t)(data[i]);
            /* Wait to be ready, buffer empty */
            UART_WAIT( base );
        }

        ret = HAL_OK;
    }

    return ret;
}

bool_t bl_uart_buff_empty ( UART_Base base )
{
    bool_t     ret = FALSE;
    Bl_Uart_t* u;

    u = bl_uart_get_handle ( base );

    /* Check if number of characters in buffer is zero */
    if (( 0 == u->num ) && ( u->in == u->out ))
    {
        ret = TRUE;
    }

    return ret;
}

bool_t bl_uart_buff_full ( UART_Base base )
{
    bool_t     ret = FALSE;
    Bl_Uart_t* u;

    u = bl_uart_get_handle ( base );

    /* Check if number of characters is the same size as buffer size */
    if ( u->num == u->size )
    {
        ret = TRUE;
    }

    return ret;
}

void bl_uart_buff_clear ( UART_Base base )
{
    Bl_Uart_t* u;

    u = bl_uart_get_handle ( base );

    /* Reset variables */
    u->num = 0;
    u->in  = 0;
    u->out = 0;
}

void bl_uart_set_custom_str_end_char ( UART_Base base, uint8_t c )
{
    Bl_Uart_t* u;

    u = bl_uart_get_handle ( base );

    u->str_delimiter = c;
}

void bl_uart_insert_to_buff ( Bl_Uart_t* u, uint8_t c )
{
    /* If there is still available space in buffer */
    if ( u->num < u->size )
    {
        /* Check overflow */
        if ( u->in == u->size )
        {
            u->in = 0;
        }
        /* Add to buffer */
        u->buff[u->in] = c;
        u->in++;
        u->num++;
    }
}

void bl_uart1_irq_hdl ( void )
{
    UART_Base base = UART1;
    /* Check if interrupt was because data is received */
    if ( base->SR & USART_SR_RXNE )
    {
        bl_uart_insert_to_buff ( &BL_UART_1, base->DR );
    }
}

void bl_uart2_irq_hdl ( void )
{
    UART_Base base = UART2;
    /* Check if interrupt was because data is received */
    if ( base->SR & USART_SR_RXNE )
    {
        bl_uart_insert_to_buff ( &BL_UART_2, base->DR );
    }
}

Bl_Uart_t* bl_uart_get_handle ( UART_Base base )
{
    Bl_Uart_t* ret = NULL;

    if ( UART1 == base )
    {
        ret = &BL_UART_1;
    }

    if ( UART2 == base )
    {
        ret = &BL_UART_2;
    }

    return ret;
}
