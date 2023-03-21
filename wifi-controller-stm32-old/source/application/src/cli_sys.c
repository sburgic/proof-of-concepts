/**
  ******************************************************************************
  * @file    application/src/cli_sys.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    17-Oct-2017
  * @brief   System CLI commands
  ******************************************************************************
  */

#include "cli.h"

static Cli_Ret cli_sys_reset ( Cli_Cmd_Args* args )
{
    Cli_Ret ret = CLI_RET_OK;
    
    ( void ) args;
    
    NVIC_SystemReset ();
    
    return ret;
}

static const Cli_Cmd sys_cmds[] = 
{
    { "reset"
    , cli_sys_reset
    , "Execute system reset"
    }
};

const Cli_Cmd_List cmd_sys_list = 
{ "sys"
, sys_cmds
, sizeof ( sys_cmds ) / sizeof ( sys_cmds[0] )
, "System commands"
};
