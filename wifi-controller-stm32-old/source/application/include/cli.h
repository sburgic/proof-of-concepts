/**
  ******************************************************************************
  * @file    application/include/cli.h
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    17-Oct-2017
  * @brief   Command Line Interface defines
  ******************************************************************************
  */

#ifndef CLI_H
#define CLI_H

#include "types.h"
#include "stm32f1xx_hal_msp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_CMD_MAX_LINE_SIZE           (64)
#define CLI_CMD_MAX_ARG                 (6)
#define CLI_CMD_MAX_ARG_SIZE            (24)
#define CLI_CMD_MAX_NAME_SIZE           CLI_CMD_MAX_ARG_SIZE
#define CLI_CMD_MAX_DESCRIPTION_SIZE    CLI_CMD_MAX_LINE_SIZE
    
/* CLI return values */
typedef enum
{
    CLI_RET_OK,
    CLI_RET_ERROR,
    CLI_RET_INV_CMD,
    CLI_RET_IGNORE_CMD,
    CLI_RET_INV_PASSWORD,
    CLI_RET_INV_HDL
} Cli_Ret;

/* CLI command argument definition */
typedef struct
{
    uint8_t  count;
    uint16_t num[CLI_CMD_MAX_ARG];
    uint8_t  str[CLI_CMD_MAX_ARG][CLI_CMD_MAX_ARG_SIZE];
} Cli_Cmd_Args;

/* CLI command function definition */
typedef Cli_Ret (*Cli_Cmd_Func)( Cli_Cmd_Args* args );

/* CLI command list structure */
typedef struct
{
    uint8_t         name[CLI_CMD_MAX_NAME_SIZE];
    Cli_Cmd_Func    cmd;
    uint8_t         description[CLI_CMD_MAX_DESCRIPTION_SIZE];
} Cli_Cmd ;

/* Command list structure */
typedef struct
{
    uint8_t         name[CLI_CMD_MAX_NAME_SIZE];
    const Cli_Cmd*  cmd_list;
    uint8_t         list_size;
    uint8_t         description[CLI_CMD_MAX_DESCRIPTION_SIZE];
} Cli_Cmd_List;

/* Entry of command table */
typedef const Cli_Cmd_List* Cli_Cmd_Table_Entry;

/* Command line structure */
typedef struct
{
    uint8_t line[CLI_CMD_MAX_LINE_SIZE];
    uint8_t cmd_len;
} Cli_Cmd_Line;

bool_t  cli_check_state ( void );
Cli_Ret cli_print_info ( void );
void    cli_start ( void );
Cli_Ret cli_login ( void );

#ifdef __cplusplus
}
#endif

#endif /* CLI_H */
