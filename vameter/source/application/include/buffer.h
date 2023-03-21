/**
 ** Name
 **   buffer.h
 **
 ** Purpose
 **   Generic cyclic buffer defines
 **
 ** Revision
 **   27-Aug-2020 (SSB) [] Initial
 **/

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "ptypes.h"

#include <stm32f1xx_hal.h>

#define BUFFER_DEFAULT_DELIMITER    ((uint8_t)'\n')

typedef struct
{
    uint32_t size;        /* Size of buffer in units of bytes */
    uint32_t in;          /* Input pointer to save next value */
    uint32_t out;         /* Output pointer to read next value */
    uint8_t* data;        /* Pointer to buffer data array */
    uint8_t  delimiter;   /* Character for string delimiter when reading
                           * from buffer as string
                           */
} Buffer_t;

/*
 * Initialize buffer data structure (Buffer_t)
 */
status_t buffer_init( Buffer_t* buff, uint32_t size, void* buff_ptr );

/*
 * Write data to buffer
 */
uint32_t buffer_write( Buffer_t* buff, const void* data, uint32_t count );

/*
 * Read data from buffer
 */
uint32_t buffer_read( Buffer_t* buff, void* data, uint32_t count );

/*
 * Get number of free elements in buffer
 */
uint32_t buffer_get_free( Buffer_t* buff );

/*
 * Get number of elements in buffer
 */
uint32_t buffer_get_full( Buffer_t* buff );

/*
 * Clear buffer pointers
 */
void buffer_reset( Buffer_t* buff );

/*
 * Check if specific element is stored in buffer
 */
int32_t buffer_find_element( Buffer_t* buff, const uint8_t element );

/*
 * Check if specific data sequence is stored in buffer
 */
int32_t buffer_find( Buffer_t* buff, const void* data, uint32_t size );

/*
 * Write string formatted data to bufferr
 */
uint32_t buffer_write_string( Buffer_t* buff, const char* string );

/*
 * Read data from buffer as string
 */
uint32_t buffer_read_string( Buffer_t* buff, char* string, uint32_t buff_size );

/*
 * Check if character exists in buffer at the given location
 */
bool_t buffer_check_element( Buffer_t* buff, uint32_t pos, void* element );

/*
 * Set string delimiter character when reading from buffer as string
 */
void buffer_set_string_delimiter( Buffer_t* buff, uint8_t delim );


Buffer_t* uart_get_buff_hdl( USART_TypeDef* uart );

#endif /* __BUFFER_H__ */
