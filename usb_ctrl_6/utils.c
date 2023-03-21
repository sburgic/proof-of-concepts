/**
 ** Name
 **   utils.c
 **
 ** Purpose
 **   Utilities
 **
 ** Revision
 **   02-Jan-2021 (SSB) [] Initial
 **/

#include "utils.h"

int16_t util_strcmp( uint8_t* s1, uint8_t* s2, uint16_t len )
{
    int16_t retval = (int16_t)0;

    if (( NULL != s1 ) && ( NULL != s2 ))
    {
        while( len )
        {
            len--;
            if (( *s1 != *s2 ) || ( 0 == *s1 ) || ( 0 == *s2 ))
            {
                retval = (int16_t)( *s1 - *s2 );
                break;
            }
            s1++;
            s2++;
        }
    }

    return retval;
}

void util_memcpy( uint8_t* dest, uint8_t* src, uint16_t len )
{
    uint16_t i;

    if (( NULL != dest ) && ( NULL != src ))
    {
        for ( i = 0; i < len; i++ )
        {
            dest[i] = src[i];
        }
    }
}

int16_t util_atoi( uint8_t* str )
{
    int16_t res = 0;
    uint8_t i;

    for ( i = 0; '\0' != str[i] && (( str[i] >= 48 ) && ( str[i] <= 57 )); i++ )
    {
        res = ( res * 10 + ( str[i] - '0' ));
    }

    return res;
}

void util_reverse( uint8_t* s, uint32_t s_len )
{
    uint8_t  c;
    uint32_t i;
    uint32_t j;

    j = ( s_len - 1 );
    
    for ( i = 0;  i < j; i++ )
    {
        c    = s[i];
        s[i] = s[j];
        s[j] = c;
        j--;
    }
}

uint32_t util_itoa( int32_t n, uint8_t* s, uint32_t s_max )
{
    uint32_t i;
    bool_t   is_minus;
    uint32_t nr;

    if ( n < 0)
    {
        is_minus = TRUE;
        nr       = (uint32_t)(-n);
    }
    else
    {
        is_minus = FALSE;
        nr       = (uint32_t)n;
    }

    i = 0;
    do
    {
        s[i] = ((uint8_t)( nr % 10 )) + '0';
        i++;
        nr /= 10;
    }
    while (( nr > 0 ) && ( i < s_max ));

    if ( 0 == nr )
    {
        if ( FALSE != is_minus )
        {
            s[i] = '-';
            i++;
        }

        s[i] = '\0';
        util_reverse( s, i );
    }

    return i;
}
