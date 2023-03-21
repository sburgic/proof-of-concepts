/**
 ** Name
 **   uart.c
 **
 ** Purpose
 **   UART routines
 **
 ** Revision
 **   27-Aug-2020 (SSB) [] Initial
 **/

#include "uart.h"

#include "buffer.h"

#include <string.h>

#ifdef __GNUC__
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

static uint8_t  uart1_received_data[64];
static uint8_t  uart2_received_data[UART_BUFFER_SIZE];

static Buffer_t uart1_buffer =
{
    .size      = 64,
    .in        = 0,
    .out       = 0,
    .data      = uart1_received_data,
    .delimiter = UART_STRING_DELIMITER
};

static Buffer_t uart2_buffer =
{
    .size      = UART_BUFFER_SIZE,
    .in        = 0,
    .out       = 0,
    .data      = uart2_received_data,
    .delimiter = UART_STRING_DELIMITER
};

static __INLINE void uart_putc( USART_TypeDef* uart, volatile char c )
{
    if ( 0 != ( uart->CR1 & USART_CR1_UE ))
    {
        UART_WAIT( uart );
        UART_WRITE_DATA( uart, (uint16_t)( c & 0x01FF ));
        UART_WAIT( uart );
    }
}

Buffer_t* uart_get_buff_hdl( USART_TypeDef* uart )
{
    Buffer_t* buff = NULL;

    if ( USART1 == uart )
    {
        buff = &uart1_buffer;
    }
    if ( USART2 == uart )
    {
        buff = &uart2_buffer;
    }

    return buff;
}

static void uart_clear_all_flags( USART_TypeDef* uart, IRQn_Type irq )
{
    UART_HandleTypeDef uart_hdl;

    uart_hdl.Instance = uart;

    __HAL_UART_CLEAR_PEFLAG( &uart_hdl );
    __HAL_UART_CLEAR_FEFLAG( &uart_hdl );
    __HAL_UART_CLEAR_NEFLAG( &uart_hdl );
    __HAL_UART_CLEAR_OREFLAG( &uart_hdl );
    __HAL_UART_CLEAR_IDLEFLAG( &uart_hdl );

    HAL_NVIC_ClearPendingIRQ( irq );
}

static void uart_insert_to_buffer( Buffer_t* buff, uint8_t ch )
{
    buffer_write( buff, &ch, 1 );
}

status_t uart_init( USART_TypeDef* uart, uint32_t baudrate )
{
    status_t           ret  = STATUS_OK;
    HAL_StatusTypeDef  hret = HAL_ERROR;
    IRQn_Type          irqn = -1;
    UART_HandleTypeDef uart_hdl;

    uart_hdl.Instance          = uart;
    uart_hdl.Init.BaudRate     = baudrate;
    uart_hdl.Init.WordLength   = UART_WORDLENGTH_8B;
    uart_hdl.Init.StopBits     = UART_STOPBITS_1;
    uart_hdl.Init.Parity       = UART_PARITY_NONE;
    uart_hdl.Init.Mode         = UART_MODE_TX_RX;
    uart_hdl.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    uart_hdl.Init.OverSampling = UART_OVERSAMPLING_16;

    hret  = HAL_UART_Init( &uart_hdl );

    if ( HAL_OK != hret )
    {
        ret = STATUS_ERROR;
    }
    else
    {
        if ( USART1 == uart )
        {
            irqn = USART1_IRQn;
        }
        else if ( USART2 == uart )
        {
            irqn = USART2_IRQn;
        }
        else if ( USART3 == uart )
        {
            irqn = USART3_IRQn;
        }

        HAL_NVIC_SetPriority ( irqn, 2, 1 );
        HAL_NVIC_EnableIRQ ( irqn );

        uart_clear_all_flags( uart, irqn );

        /* Enable RX interrupt */
        uart->CR1 |= USART_CR1_RXNEIE;

        /* Enable USART peripheral */
        uart->CR1 |= USART_CR1_UE;

    }

    return ret;
}

void uart_puts( USART_TypeDef* uart, char* str )
{
    while ( *str )
    {
        UART_WAIT(uart);
        UART_WRITE_DATA(uart, (uint16_t)(*str));
        UART_WAIT(uart);

        str++;
    }
}

void uart_send( USART_TypeDef* uart, uint8_t* data, uint16_t count )
{
    while ( count > 0 )
    {
        UART_WAIT(uart);
        UART_WRITE_DATA(uart, (uint16_t)(*data));
        UART_WAIT(uart);

        data++;
        count--;
    }
}

uint8_t uart_getc( USART_TypeDef* uart )
{
    uint8_t   ch;
    uint32_t  t_ret;
    Buffer_t* buff;

    buff  = uart_get_buff_hdl( uart );
    t_ret = buffer_read( buff, &ch, 1 );

    if ( 0 == t_ret )
    {
        ch = 0;
    }

    return ch;
}

uint16_t uart_gets( USART_TypeDef* uart, char* data, uint16_t buff_size )
{
    uint16_t  ret;
    Buffer_t* buff;

    buff = uart_get_buff_hdl( uart );
    ret  = (uint16_t) buffer_read_string( buff, data, buff_size );

    return ret;
}

int16_t uart_find_char( USART_TypeDef* uart, uint8_t ch )
{
    int16_t   ret;
    Buffer_t* buff;

    buff = uart_get_buff_hdl( uart );

    ret = (int16_t) buffer_find_element( buff, ch );

    return ret;
}

bool_t uart_buff_empty( USART_TypeDef* uart )
{
    bool_t    ret = TRUE;
    uint32_t  full;
    Buffer_t* buff;

    buff = uart_get_buff_hdl( uart );
    full = buffer_get_full( buff );

    if ( 0 != full )
    {
        ret = FALSE;
    }

    return ret;
}

bool_t uart_buff_full( USART_TypeDef* uart )
{
    bool_t    ret = FALSE;
    uint32_t  free;
    Buffer_t* buff;

    buff = uart_get_buff_hdl( uart );
    free = buffer_get_free( buff );

    if ( 0 == free )
    {
        ret = TRUE;
    }

    return ret;
}

uint16_t uart_buff_count( USART_TypeDef* uart )
{
    uint16_t  ret = 0;
    Buffer_t* buff;

    buff = uart_get_buff_hdl( uart );
    ret  = (uint16_t) buffer_get_full( buff );

    return ret;
}

void uart_clear_buff( USART_TypeDef* uart )
{
    Buffer_t* buff;

    buff = uart_get_buff_hdl( uart );
    buffer_reset( buff );
}

void uart_set_custom_string_delimiter( USART_TypeDef* uart, uint8_t delim )
{
    Buffer_t* buff;

    buff = uart_get_buff_hdl( uart );
    buffer_set_string_delimiter( buff, delim );
}

int16_t uart_find_string( USART_TypeDef* uart, char* str )
{
    int16_t   ret;
    Buffer_t* buff;

    buff = uart_get_buff_hdl( uart );
    ret  = (int16_t) buffer_find( buff
                                , (uint8_t*) str
                                , strnlen( str, UART_BUFFER_SIZE )
                                );

    return ret;
}

void USART2_IRQHandler( void )
{
    /* Check if interrupt occurred because data is received */
    if ( 0 != ( USART2->SR & USART_SR_RXNE ))
    {
        uart_insert_to_buffer( &uart2_buffer, UART_READ_DATA( USART2 ));
    }

    uart_clear_all_flags( USART2, USART2_IRQn );
}

void USART1_IRQHandler( void )
{
    /* Check if interrupt occurred because data is received */
    if ( 0 != ( USART1->SR & USART_SR_RXNE ))
    {
        uart_insert_to_buffer( &uart1_buffer, UART_READ_DATA( USART1 ));
    }

    uart_clear_all_flags( USART1, USART1_IRQn );
}

void HAL_UART_MspInit( UART_HandleTypeDef* huart )
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if ( USART1 == huart->Instance )
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_AFIO_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin   = GPIO_PIN_9;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

        GPIO_InitStruct.Pin  = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
    }
    else if ( USART2 == huart->Instance )
    {
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_AFIO_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin   = GPIO_PIN_2;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

        GPIO_InitStruct.Pin  = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
    }
}

PUTCHAR_PROTOTYPE
{
    uart_putc( UART_TO_PC, ch );
    return ch;
}
