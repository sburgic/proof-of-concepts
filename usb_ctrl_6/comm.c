/**
 ** Name
 **   comm.c
 **
 ** Purpose
 **   Communication API
 **
 ** Revision
 **   06-Feb-2021 (SSB) [] Initial
 **/

#include "comm.h"

#include "time.h"
#include "utils.h"

static uint8_t comm_buffer[COMM_BUFF_SIZE] = {0};
static bool_t  comm_supervision_enable     = FALSE;

uint8_t comm_get_output_state( uint8_t output )
{
    uint8_t state = 0xFF;

    if ( 1 == output )
    {
        state = RELAY1;
    }
    else if ( 2 == output )
    {
        state = RELAY2;
    }
    else if ( 3 == output )
    {
        state = RELAY3;
    }
    else if ( 4 == output )
    {
        state = RELAY4;
    }
    else if ( 5 == output )
    {
        state = RELAY5;
    }
    else if ( 6 == output )
    {
        state = RELAY6;
    }

    return state;
}

status_t comm_set_output_state( uint8_t output, uint8_t new_state )
{
    status_t ret = STATUS_OK;

    if ( new_state < 2 )
    {
        switch ( output )
        {
            case 1:
                RELAY1 = new_state;
                break;
            case 2:
                RELAY2 = new_state;
                break;
            case 3:
                RELAY3 = new_state;
                break;
            case 4:
                RELAY4 = new_state;
                break;
            case 5:
                RELAY5 = new_state;
                break;
            case 6:
                RELAY6 = new_state;
                break;
            default:
                ret = STATUS_ERROR;
                break;
        }
    }
    else
    {
        ret = STATUS_ERROR;
    }

    return ret;
}

void comm_output_state_to_string( uint8_t*  buff
                                , uint16_t* len
                                , uint8_t   output
                                , uint8_t   state
                                )
{
    if (( NULL != buff ) && ( NULL != len ))
    {
        if (( output > 0 ) && ( output < 7 ))
        {
            *len = 5;
            
            util_memcpy( buff, (uint8_t*)"OUT", 3 );
            buff[3] = output + 48;
            buff[4] = ':';

            util_memcpy( &buff[*len]
                       , ( state == 1 ) ? \
                         (uint8_t*)"1" : (uint8_t*)"0"
                       , 1
                       );
            *len += 1;

            buff[*len] = '\0';
        }
    }
}

bool_t comm_process_msg( uint8_t* msg, uint16_t* len, uint16_t offset )
{
    bool_t  ret = FALSE;
    uint8_t state;
    uint8_t io;

    if (( NULL != msg ) && ( *len > 0 ))
    {
        if ( 'G' == msg[offset])
        {
            if ( 'O' == msg[offset + 1])
            {
                io    = msg[offset + 2] - '0';
                state = comm_get_output_state( io );

                if ( 0xFF != state )
                {
                    comm_output_state_to_string( msg
                                               , len
                                               , io
                                               , state
                                               );

                    ret = TRUE;
                }
            }
        }
        else if ( 'S' == msg[offset])
        {
            if ( 'O' == msg[offset + 1])
            {
                io    = msg[offset + 2] - '0';
                state = msg[offset + 3] - '0';
                comm_set_output_state( io
                                     , state
                                     );
            }
            if (( 'S' == msg[offset + 1] ) && ( 'V' == msg[offset + 2] ))
            {
                comm_supervision_enable = (bool_t)( msg[offset + 3] - '0' );
            }
        }
    }

    return ret;
}

uint8_t* comm_get_buff_hdl( void )
{
    return &comm_buffer[0];
}

bool_t comm_get_sv_state( void )
{
    return comm_supervision_enable;
}
