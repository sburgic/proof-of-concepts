/**
 ** Name
 **   state_machine.h
 **
 ** Purpose
 **   Application operational state definitons
 **
 ** Revision
 **   10-Oct-2020 (SSB) [] Initial
 **/

#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

typedef enum
{
    STATE_MACHINE_NORMAL = 0,
    STATE_MACHINE_CLI,
    STATE_MACHINE_ERROR
} State_Machine_t;

State_Machine_t sm_get_state( void );
void sm_set_state( State_Machine_t state );

#endif /* __STATE_MACHINE_H__ */
