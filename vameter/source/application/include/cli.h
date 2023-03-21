/**
 ** Name
 **   cli.h
 **
 ** Purpose
 **   Command Line Interface
 **
 ** Revision
 **   14-May-2020 (SSB) [] Initial
 **/

#ifndef __CLI_H__
#define __CLI_H__

#include "ptypes.h"

#define CLI_UART USART1

#define CLI_CMD_MAX_LINE_SIZE        (64)
#define CLI_CMD_MAX_ARG              (6)
#define CLI_CMD_MAX_ARG_SIZE         (24)
#define CLI_CMD_MAX_NAME_SIZE        CLI_CMD_MAX_ARG_SIZE
#define CLI_CMD_MAX_DESCRIPTION_SIZE CLI_CMD_MAX_LINE_SIZE

typedef enum
{
    CLI_RET_OK,
    CLI_RET_ERROR,
    CLI_RET_INV_CMD,
    CLI_RET_IGNORE_CMD,
    CLI_RET_INV_PASSWORD,
    CLI_RET_INV_HDL
} Cli_Ret;

typedef struct
{
    uint8_t  count;
    uint16_t num[CLI_CMD_MAX_ARG];
    uint8_t  str[CLI_CMD_MAX_ARG][CLI_CMD_MAX_ARG_SIZE];
} Cli_Cmd_Args;

typedef Cli_Ret (*Cli_Cmd_Func)( Cli_Cmd_Args* args );

typedef struct
{
    uint8_t         name[CLI_CMD_MAX_NAME_SIZE];
    Cli_Cmd_Func    cmd;
    uint8_t         description[CLI_CMD_MAX_DESCRIPTION_SIZE];
} Cli_Cmd ;

typedef struct
{
    uint8_t         name[CLI_CMD_MAX_NAME_SIZE];
    const Cli_Cmd*  cmd_list;
    uint8_t         list_size;
    uint8_t         description[CLI_CMD_MAX_DESCRIPTION_SIZE];
} Cli_Cmd_List;

typedef const Cli_Cmd_List* Cli_Cmd_Table_Entry;

typedef struct
{
    uint8_t line[CLI_CMD_MAX_LINE_SIZE];
    uint8_t cmd_len;
} Cli_Cmd_Line;

void cli_process( void );

#endif /* __CLI_H__ */
