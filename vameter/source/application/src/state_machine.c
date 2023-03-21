/**
 ** Name
 **   state_machine.c
 **
 ** Purpose
 **   Application operational state routines
 **
 ** Revision
 **   30-Aug-2020 (SSB) [] Initial
 **/

#include "state_machine.h"

static volatile State_Machine_t app_state = STATE_MACHINE_NORMAL;

State_Machine_t sm_get_state( void )
{
    return app_state;
}

void sm_set_state( State_Machine_t state )
{
    app_state = state;
}
