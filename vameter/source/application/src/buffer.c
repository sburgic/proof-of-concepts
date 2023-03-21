/**
 ** Name
 **   buffer.c
 **
 ** Purpose
 **   Generic cyclic buffer routines
 **
 ** Revision
 **   27-Aug-2020 (SSB) [] Initial
 **/

#include "buffer.h"

#include <string.h>

status_t buffer_init( Buffer_t* buff, uint32_t size, void* buff_ptr )
{
    status_t ret = STATUS_ERROR;

    if (( NULL != buff ) && ( NULL != buff_ptr ))
    {

        /* Set all values to zero */
        memset( buff, 0, sizeof( Buffer_t ));

        /* Set default values */
        buff->size      = size;
        buff->data      = buff_ptr;
        buff->delimiter = BUFFER_DEFAULT_DELIMITER;

        ret = STATUS_OK;
    }

    return ret;
}

uint32_t buffer_write( Buffer_t* buff, const void* data, uint32_t count )
{
    uint32_t ret = 0;
    uint32_t i   = 0;
    uint32_t free;
    uint32_t tocopy;
    uint8_t* d;

    if ((( NULL != buff ) && ( NULL != data )) && ( count > 0 ))
    {
        d = (uint8_t*) data;

        /* Check input pointer */
        if ( buff->in >= buff->size )
        {
            buff->in = 0;
        }

        free = buffer_get_free( buff );

        if ( free != 0 )
        {
            if ( free < count )
            {
                count = free;
            }

            /* Calculate number of elements to be put at the end of buffer */
            tocopy = buff->size - buff->in;

            if ( tocopy > count )
            {
                tocopy = count;
            }

            memcpy( &buff->data[buff->in], data, tocopy );

            i        += tocopy;
            buff->in += tocopy;
            count   -= tocopy;

            if ( count > 0 )
            {
                /* Start from the beginning of buffer*/
                buff->in = 0;

                memcpy( &buff->data[buff->in], &d[i], count );

                buff->in = count;
            }

            if ( buff->in >= buff->size )
            {
                buff->in = 0;
            }

            /* Return number of elements stored in memory */
            ret = i + count;
        }
    }

    return ret;
}

uint32_t buffer_read( Buffer_t* buff, void* data, uint32_t count )
{
    uint32_t ret = 0;
    uint32_t i   = 0;
    uint32_t full;
    uint32_t tocopy;
    uint8_t* d;

    if ((( NULL != buff ) && ( NULL != data )) && ( count > 0 ))
    {
        d = (uint8_t*) data;

        /* Check output pointer */
        if ( buff->out >= buff->size )
        {
            buff->out = 0;
        }

        full = buffer_get_full( buff );

        if ( full != 0 )
        {
            if ( full < count )
            {
                count = full;
            }

            tocopy = buff->size - buff->out;

            if ( tocopy > count )
            {
                tocopy = count;
            }

            memcpy( d, &buff->data[buff->out], tocopy );

            i         += tocopy;
            buff->out += tocopy;
            count     -= tocopy;

            if ( count > 0 )
            {
                buff->out = 0;

                memcpy( &d[i],  &buff->data[buff->out], count );

                buff->out = count;
            }

            if ( buff-> out >= buff->size )
            {
                buff->out = 0;
            }

            ret = i + count;
        }
    }

    return ret;
}

uint32_t buffer_get_free( Buffer_t* buff )
{
    uint32_t size = 0;
    uint32_t in;
    uint32_t out;

    if ( NULL != buff )
    {
        in  = buff->in;
        out = buff->out;

        if ( out > in )
        {
            size = out - in;
        }
        else if ( in > out )
        {
            size = buff->size - ( in - out );
        }
        else
        {
            size = buff->size;
        }
    }

    return ( size - 1 );
}

uint32_t buffer_get_full( Buffer_t* buff )
{
    uint32_t size = 0;
    uint32_t in;
    uint32_t out;

    if ( NULL != buff )
    {
        in  = buff->in;
        out = buff->out;

        if ( in > out )
        {
            size = in - out;
        }
        else if ( out > in )
        {
            size = buff->size - ( out - in );
        }
        else
        {
            size = 0;
        }
    }

    return size;
}

void buffer_reset( Buffer_t* buff )
{
    if ( NULL != buff )
    {
        buff->in  = 0;
        buff->out = 0;
    }
}

int32_t buffer_find_element( Buffer_t* buff, const uint8_t element )
{
    int32_t  ret = -1;
    uint32_t num;
    uint32_t out;

    if ( NULL != buff )
    {
        num = buffer_get_full( buff );
        out = buff->out;

        while ( num > 0 )
        {
            if ( out >= buff->size )
            {
                out = 0;
            }

            if ((uint8_t) buff->data[out] == (uint8_t) element )
            {
                break;
            }

            out++;
            num--;
            ret++;
        }
    }

    return ret;
}

int32_t buffer_find( Buffer_t* buff, const void* data, uint32_t size )
{
    int32_t  ret = -1;
    uint32_t num;
    uint32_t out;
    uint32_t i;
    uint8_t  found = 0;
    uint8_t* d;

    if (( NULL != buff ) && ( NULL != data ))
    {
        num = buffer_get_full( buff );

        if ( num >= size )
        {
            d = (uint8_t*) data;
            out = buff->out;

            while ( num > 0 )
            {
                if ( out >= buff->size )
                {
                    out = 0;
                }

                if ((uint8_t) buff->data[out] == (uint8_t) d[0] )
                {
                    found = 1;
                }

                out++;
                num--;
                ret++;

                if ( 0 != found )
                {
                    i = 1;

                    while (( i < size ) && ( num > 0 ))
                    {
                        if ( out >= buff->size )
                        {
                            out = 0;
                        }

                        if ((uint8_t) buff->data[out] != (uint8_t) d[i])
                        {
                            ret += i - 1;
                            break;
                        }

                        out++;
                        num--;
                        i++;
                    }

                    if ( i == size )
                    {
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

uint32_t buffer_write_string( Buffer_t* buff, const char* string )
{
    return buffer_write( buff, (uint8_t *)string, strlen( string ));
}

uint32_t buffer_read_string( Buffer_t* buff, char* string, uint32_t buff_size )
{
    uint32_t ret = 0;
    uint32_t i   = 0;
    uint32_t mem_free;
    uint32_t mem_full;
    int32_t  delim_found;
    uint8_t  ch;

    if ((( NULL != buff ) && ( NULL != string )) && ( 0 != buff_size ))
    {
        mem_free = buffer_get_free( buff );
        mem_full = buffer_get_full( buff );

        delim_found = buffer_find_element( buff, buff->delimiter );

        if (( 0 != mem_full )
         && ( delim_found >= 0 )
         && ( mem_full >= buff_size )
         && ( mem_free > 0 ))
        {
            while ( i < ( buff_size - 1 ))
            {
                ret = buffer_read( buff, &ch, 1 );

                if ( 0 == ret )
                {
                    break;
                }

                string[i] = (char) ch;

                if ((char) string[i] == (char) buff->delimiter )
                {
                    break;
                }

                i++;
            }

            if ( i == ( buff_size - 1 ))
            {
                string[i] = 0;
            }
            else
            {
                i++;
                string[i] = 0;
            }
        }
    }

    return ret;
}

bool_t buffer_check_element( Buffer_t* buff, uint32_t pos, void* element )
{
    bool_t   ret = FALSE;
    uint32_t i   = 0;
    uint32_t in;
    uint32_t out;

    if (( NULL != buff ) && ( NULL != element ))
    {
        in  = buff->in;
        out = buff->out;

        while (( i < pos ) && ( in != out ))
        {
            out++;
            i++;

            if ( out >= buff->size )
            {
                out = 0;
            }
        }

        if ( i == pos )
        {
            *(uint8_t *) element = buff->data[out];

            ret = TRUE;
        }
    }

    return ret;
}

void buffer_set_string_delimiter( Buffer_t* buff, uint8_t delim )
{
    if ( NULL != buff )
    {
        buff->delimiter = delim;
    }
}
