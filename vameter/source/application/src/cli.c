/**
 ** Name
 **   cli.c
 **
 ** Purpose
 **   Command Line Interface
 **
 ** Revision
 **   28-Aug-2020 (SSB) [] Initial
 **/

#include "cli.h"

#include "uart.h"

#include <stdlib.h>
#include <string.h>

#define CLI_CMD_BUFF_NUM (2)

extern const Cli_Cmd_List cmd_sys_list;

static const Cli_Cmd_Table_Entry cli_cmd_table[] =
{
    &cmd_sys_list
};

static void cli_fill_with_space( uint8_t name_size )
{
    uint8_t space_size = 15;
    uint8_t i;
    uint8_t out_space[16];

    out_space[0] = '\0';

    if ( name_size >= space_size )
    {
        space_size = 1;
    }
    else
    {
        space_size -= name_size;
    }

    for ( i = 0; i < space_size; i++ )
    {
        strcat( (char*)out_space, " " );
    }

    printf( "%s", out_space );
}

static void cli_print_help( void )
{
    uint8_t table_size = sizeof ( cli_cmd_table )
                       / sizeof ( Cli_Cmd_Table_Entry );
    uint8_t  i;

    for ( i = 0; i < table_size; i++ )
    {
        printf( "%s", cli_cmd_table[i]->name );

        cli_fill_with_space( strnlen((char*) cli_cmd_table[i]->name
                                    , CLI_CMD_MAX_NAME_SIZE
                                    )
                           );

        printf( "%s\r\n", cli_cmd_table[i]->description );
    }

    printf( "\r\n" );
}

static void cli_delete_others( uint8_t num )
{
    uint8_t i;

    for ( i = 0 ; i < num; i++ )
    {
        uart_send( CLI_UART, (uint8_t*)" ", 1 );
    }

    for ( i = 0; i< num; i++ )
    {
        uart_send( CLI_UART, (uint8_t*)"\b", 1 );
    }
}

static uint32_t cli_read_line( uint8_t* buff
                             , uint8_t  max_len
                             )
{
    static Cli_Cmd_Line cmd_buff[CLI_CMD_BUFF_NUM];

    uint8_t      cmd_delta = 0;
    uint8_t      cmd_ind   = 0;
    uint8_t      cmd_num   = 0;
    uint8_t      i = 0;
    uint8_t      ch;
    bool_t       going = TRUE;
    bool_t       is_empty;
    int8_t       ind;
    int8_t       del;

    while (( i < max_len ) && ( FALSE != going ))
    {
        is_empty = uart_buff_empty( CLI_UART );

        if ( FALSE == is_empty )
        {
            ch = uart_getc( CLI_UART );

            switch ( ch )
            {
                case 0x0A:  /**< Line feed \n */
                case 0x0D:  /**< Carriage return \r */
                    going = FALSE;

                    if ( i > 1 )
                    {
                        memcpy( cmd_buff[cmd_ind].line, buff, i );
                        cmd_buff[cmd_ind].cmd_len = i;
                        cmd_ind++;

                        if ( CLI_CMD_BUFF_NUM == cmd_num )
                        {
                            cmd_ind = 0;
                        }

                        if ( cmd_num < CLI_CMD_BUFF_NUM )
                        {
                            cmd_ind++;
                        }

                        cmd_delta = 0;
                    }

                    buff[i] = 0x0;
                    printf( "\r\n" );
                    break;

                case 0x09:  /**< Horizontal tab */
                case 0x08:  /**< Backspace */
                case 0x7F:  /**< DEL */
                    if ( 0 != i )
                    {
                        i--;
                        uart_send( CLI_UART, (uint8_t*)"\b", 1 );
                    }
                    break;

                case 0x41:
                    if ( ( 0x5B == buff[i-1] ) && ( 0x1B == buff[i-2] ) )
                    {
                        uart_send( CLI_UART, (uint8_t*)"\r> ", 3 );

                        if ( cmd_delta < cmd_num )
                        {
                            cmd_delta++;
                        }

                        ind = ( cmd_ind - cmd_delta ) % CLI_CMD_BUFF_NUM;
                        memcpy ( buff
                               , cmd_buff[ind].line
                               , cmd_buff[ind].cmd_len
                               );

                        del = i - cmd_buff[ind].cmd_len - 2;
                        i   = cmd_buff[ind].cmd_len;

                        uart_send( CLI_UART, buff, i );

                        if ( del > 0 )
                        {
                            cli_delete_others( del );
                        }
                    }
                    break;

                case 0x42:
                    if ( ( 0x5B == buff[i-1] ) && ( 0x1B == buff[i-2] ) )
                    {
                        uart_send( CLI_UART, (uint8_t*)"\r> ", 3 );

                        if ( cmd_delta > 0 )
                        {
                            cmd_delta--;
                        }

                        ind = ( cmd_ind - cmd_delta ) % CLI_CMD_BUFF_NUM;
                        memcpy ( buff
                               , cmd_buff[ind].line
                               , cmd_buff[ind].cmd_len
                               );

                        del = i - cmd_buff[ind].cmd_len - 2;
                        i   = cmd_buff[ind].cmd_len;
                        uart_send ( CLI_UART, buff, i );

                        if ( del > 0 )
                        {
                            cli_delete_others( del );
                        }
                    }
                    break;

                default:
                    buff[i++] = ch;

                    if ( ( 0x5B != ch ) && ( 0x1B != ch ) )
                    {
                        uart_send( CLI_UART, &ch, 1 );
                    }
            }
        }
    }

    return i;
}

static void cli_parse_cmd( Cli_Cmd_Args* args, uint8_t* str )
{
    char     delims[] = " ";
    char*    result = NULL;
    uint8_t  i = 0;

    result = strtok ( (char*)str, delims );

    while ( NULL != result )
    {
        if ( strnlen( result , CLI_CMD_MAX_ARG_SIZE ) >=
            CLI_CMD_MAX_ARG_SIZE )
        {
            printf( "Error: Argument %s is too long!\r\n", result );
            break;
        }

        strncpy( (char*) args->str[i]
               , result
               , CLI_CMD_MAX_ARG_SIZE
               );

        if ( ( '0' == result[0] ) && ( 'x' == result[1] ) )
        {
            args->num[i] = strtoul( result, NULL, 16 );
        }
        else
        {
            args->num[i] = atoi( result );
        }

        result = (char*) strtok( NULL, delims );
        i++;

        if ( i >= CLI_CMD_MAX_ARG )
        {
            printf( "Error: Maximal argument count reached!\r\n" );
            break;
        }
    }

    args->count = i;
}

static void cli_print_help_sub_cmd( Cli_Cmd_List* subcmds )
{
    uint8_t size;
    uint8_t i;

    size = subcmds->list_size;

    for ( i = 0; i < size; i++ )
    {
        printf( "%s %s", subcmds->name, subcmds->cmd_list[i].name );

        cli_fill_with_space( strnlen( (char*) subcmds->name
                                    , CLI_CMD_MAX_NAME_SIZE
                                    )
                           + strnlen( (char*) subcmds->cmd_list[i].name
                                    , CLI_CMD_MAX_NAME_SIZE
                                    )
                           + 1
                           );

        printf( "%s\r\n", subcmds->cmd_list[i].description );
    }
}

static Cli_Ret cli_run_cmd ( Cli_Cmd_Args* args )
{
    Cli_Ret  ret = CLI_RET_INV_CMD;
    uint16_t table_size = sizeof ( cli_cmd_table )
                        / sizeof ( Cli_Cmd_Table_Entry );
    uint8_t     i = 0;
    uint8_t     j = 0;
    uint8_t     size = 0;
    Cli_Cmd*    commands;

    if ( 0 == args->count )
    {
        ret = CLI_RET_IGNORE_CMD;
    }

    if ( CLI_RET_IGNORE_CMD != ret )
    {
        if ( 0 == strncmp((char*) args->str[0]
                         , "help"
                         , CLI_CMD_MAX_LINE_SIZE ) ||
           ( 0 == strncmp((char*) args->str[0]
                         , "?"
                         , CLI_CMD_MAX_LINE_SIZE )))
        {
            cli_print_help ();

            ret = CLI_RET_OK;
        }
    }

    if ( ( CLI_RET_OK != ret ) && ( CLI_RET_IGNORE_CMD != ret ) )
    {
        while ( i < table_size )
        {
            if ( 0 == strncmp ((char*) args->str[0]
                              , (char*) cli_cmd_table[i]->name
                              , CLI_CMD_MAX_LINE_SIZE ))
            {
                commands = (Cli_Cmd*) cli_cmd_table[i]->cmd_list;
                size     = cli_cmd_table[i]->list_size;

                if ( 1 == args->count )
                {
                    cli_print_help_sub_cmd((Cli_Cmd_List*) cli_cmd_table[i] );
                    ret = CLI_RET_OK;
                    break;
                }
                else
                {
                    if ( 0 == strncmp((char*) args->str[1]
                                     , "help"
                                     , CLI_CMD_MAX_LINE_SIZE ) ||
                       ( 0 == strncmp((char*) args->str[1]
                                     , "?"
                                     , CLI_CMD_MAX_LINE_SIZE )))
                    {
                        cli_print_help_sub_cmd
                                ((Cli_Cmd_List*) cli_cmd_table[i] );
                        ret = CLI_RET_OK;
                        break;
                    }

                    for ( j = 0; j < size; j++ )
                    {
                        if ( 0 == strncmp((char*) args->str[1]
                                         , (char*) commands[j].name
                                         , CLI_CMD_MAX_LINE_SIZE ) )
                        {
                            ret = commands[j].cmd ( args );
                            break;
                        }

                        if (( size - 1 ) == j )
                        {
                            cli_print_help_sub_cmd
                                    ((Cli_Cmd_List*) cli_cmd_table[i] );
                            ret = CLI_RET_INV_CMD;
                        }
                    }
                }
            }
            i++;
        }
    }
    return ret;
}

void cli_process ( void )
{
    uint8_t         cmd_str[CLI_CMD_MAX_LINE_SIZE] = {0};
    Cli_Cmd_Args    args = {0};

    cli_print_help();

    while ( TRUE )
    {
        uart_send( CLI_UART, (uint8_t*)"> ", 2 );
        cli_read_line( cmd_str, CLI_CMD_MAX_LINE_SIZE );
        cli_parse_cmd( &args, cmd_str );
        cli_run_cmd( &args );
    }
}
