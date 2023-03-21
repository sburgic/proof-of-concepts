/**
 ** Name
 **   comm.h
 **
 ** Purpose
 **   Communication API
 **
 ** Revision
 **   06-Feb-2021 (SSB) [] Initial
 **/

#ifndef __COMM_H__
#define __COMM_H__

#include "ptypes.h"

#include <htc.h>
#include <pic.h>

#define RELAY6 RB3
#define RELAY5 RB0
#define RELAY4 RA1
#define RELAY3 RA0
#define RELAY2 RB5
#define RELAY1 RB4

#define COMM_BUFF_SIZE 128

uint8_t comm_get_output_state( uint8_t output );
status_t comm_set_output_state( uint8_t output, uint8_t new_state );
void comm_output_state_to_string( uint8_t*  buff
                                , uint16_t* len
                                , uint8_t   output
                                , uint8_t   state
                                );
bool_t comm_process_msg( uint8_t* msg, uint16_t* len, uint16_t offset );
uint8_t* comm_get_buff_hdl( void );
bool_t comm_get_sv_state( void );

#endif /* __COMM_H__ */
