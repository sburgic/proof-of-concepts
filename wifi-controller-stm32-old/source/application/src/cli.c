/**
  ******************************************************************************
  * @file    application/src/cli.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    17-Oct-2017
  * @brief   Command Line Interface
  ******************************************************************************
  */

#include "cli.h"

#include "lock.h"
#include "bl_uart.h"

#include "bsp_wifi_controller.h"

#include <sl_handle.h>
#include <sl_string.h>
#include <sl_mem.h>

#include <string.h>
#include <stdlib.h>

#define CLI_UART UART_DBG

#define CLI_CMD_BUFF_NUM        (2)
#define CLI_PASSWORD_MAX_RETRY  (2)

extern const Cli_Cmd_List cmd_led_list;
extern const Cli_Cmd_List cmd_wifi_list;
extern const Cli_Cmd_List cmd_sys_list;

static uint8_t cli_pass[] = { 'd', 'z', 'e', 'v', 'l', 'a', '\0' };

static const Cli_Cmd_Table_Entry cli_cmd_table[] =
{
    &cmd_wifi_list
  , &cmd_sys_list
};

/* Print CLI usage instructions */
static void cli_print_help ( void );
/* Print sub commands usage */
static void cli_print_help_sub_cmd ( Cli_Cmd_List* subcmds );
/* Read line from UART */
static uint32_t cli_read_line ( uint8_t*    buff
                              , uint8_t     max_len
                              );
/* Parse entered command */
static void cli_parse_cmd ( Cli_Cmd_Args* args, uint8_t* buff );
/* Run parsed command */
static Cli_Ret cli_run_cmd ( Cli_Cmd_Args* args );

static void cli_delete_others ( uint8_t num );

static void cli_fill_with_space ( uint8_t name_size );


bool_t cli_check_state ( void )
{
    bool_t   ret = TRUE;
    uint32_t pin_state;
   
    pin_state = bsp_pb_get_state ( S2_BUTTON );
    
    if ( ZERO != pin_state )
    {
        ret = FALSE;
    }
    
    return ret;    
}

Cli_Ret cli_print_info ( void )
{
    Cli_Ret ret = CLI_RET_OK;
    HAL_Ret hret;

    hret = bl_uart_send ( CLI_UART
                        , (uint8_t*)"\nCLI started...\r\n\n"
                        , 18
                        );
        
    if ( HAL_OK != hret )
    {
        ret = CLI_RET_ERROR;
    }
    else
    {
        cli_print_help ();
    }
    
    return ret;
}

void cli_start ( void )
{
    uint8_t         cmd_str[CLI_CMD_MAX_LINE_SIZE] = {0};
    Cli_Cmd_Args    args = {0};
        
    while ( TRUE )
    {
        bl_uart_send ( CLI_UART, (uint8_t*)"> ", 2 );
        cli_read_line ( cmd_str, CLI_CMD_MAX_LINE_SIZE );
        cli_parse_cmd ( &args, cmd_str );
        cli_run_cmd ( &args );
    }
}

Cli_Ret cli_login ( void )
{
    Cli_Ret  ret = CLI_RET_INV_PASSWORD;
    bool_t   is_empty;
    uint8_t  rec[SL_MAX_STRING_SIZE] = {0};
    uint8_t  out[SL_MAX_STRING_SIZE] = {0};
    uint8_t  ch;
    uint8_t  i;
    uint32_t no_of_retries_left;
    uint8_t  rec_ch_idx = 0;
    bool_t   going = TRUE;

    bl_uart_send ( CLI_UART
                 , (uint8_t*)"Enter password to access CLI:\r\n"
                 , 31
                 );

    no_of_retries_left = lock_get_no_of_retries();

    /* In case the flash is damaged allow application to continue */
    if ( no_of_retries_left > CLI_PASSWORD_MAX_RETRY )
    {
        no_of_retries_left = CLI_PASSWORD_MAX_RETRY;
    }

    i = CLI_PASSWORD_MAX_RETRY - no_of_retries_left;

    do
    {
        while ( ( rec_ch_idx < SL_MAX_STRING_SIZE ) && ( FALSE != going ) )
        {
            is_empty = bl_uart_buff_empty( CLI_UART );
            
            if ( FALSE == is_empty )
            {
                ch = bl_uart_getc( CLI_UART );

                switch ( ch )
                {
                    case 0x0A:
                    case 0x0D:
                        rec[rec_ch_idx] = '\0';
                        going = FALSE;
                        break;
                    
                    default:
                        rec[rec_ch_idx] = ch;
                        rec_ch_idx++;
                        bl_uart_send ( CLI_UART, (uint8_t*)"*", 1 );
                }
            }
        }

        if ( ZERO != sl_strncmp ( rec, cli_pass, SL_MAX_STRING_SIZE ) )
        {
            if ( CLI_PASSWORD_MAX_RETRY == i )
            {
                bl_uart_send ( CLI_UART
                             , (uint8_t*)"\r\nWrong password.\r\n"
                             , 19
                             );
                
                lock_device ();
                NVIC_SystemReset ();
            }
            else
            {
                sl_sprintf_d ( out
                             , (uint8_t*)"\n\rWrong password, "
                                         "%d attempt(s) left\n\r"
                                         "Enter password: \n\r"
                             , (int32_t)( CLI_PASSWORD_MAX_RETRY - i )
                             , sizeof ( out )
                             );
            
                bl_uart_send ( CLI_UART
                             , out
                             , sl_strnlen ( out, SL_MAX_STRING_SIZE )
                             );

                lock_update_no_of_retries( CLI_PASSWORD_MAX_RETRY - i - 1);
            }
        }
        else
        {
            lock_update_no_of_retries( CLI_PASSWORD_MAX_RETRY );
            ret = CLI_RET_OK;
            break;
        }
        i++;
        rec_ch_idx = 0;
        going = TRUE;
    }
    while ( i <= CLI_PASSWORD_MAX_RETRY );
    
    return ret;
}

static void cli_print_help ( void )
{
    uint8_t table_size = sizeof ( cli_cmd_table )
                       / sizeof ( Cli_Cmd_Table_Entry ); 
    uint8_t  i;
    uint8_t  out[SL_MAX_STRING_SIZE] = {0};
    
    for ( i = 0; i < table_size; i++ )
    {
        sl_sprintf_s ( out
                     , (uint8_t*)"%s"
                     , (uint8_t*)cli_cmd_table[i]->name
                     , sizeof ( out )
                     );
        
        bl_uart_send ( CLI_UART, out, sl_strnlen(out, SL_MAX_STRING_SIZE) );
        
        cli_fill_with_space ( sl_strnlen ( cli_cmd_table[i]->name
                            , CLI_CMD_MAX_NAME_SIZE
                            )
                            );
        
        sl_sprintf_s ( out
                     , (uint8_t*)"%s\r\n"
                     , cli_cmd_table[i]->description
                     , sizeof ( out )
                     );
                     
        bl_uart_send ( CLI_UART, out, sl_strnlen(out, SL_MAX_STRING_SIZE) );
    }
    
    bl_uart_send ( CLI_UART, (uint8_t*)"\n", 1 );
    
}

static uint32_t cli_read_line ( uint8_t*    buff
                              , uint8_t     max_len
                              )
{
    static Cli_Cmd_Line cmd_buff[CLI_CMD_BUFF_NUM];
    
    uint8_t      cmd_delta = 0;
    uint8_t      cmd_ind = 0;
    uint8_t      cmd_num = 0;
    uint8_t      i = 0;
    uint8_t      ch;
    bool_t       going = TRUE;
    bool_t       is_empty;
    int8_t       ind;
    int8_t       del;
    
    while ( ( i < max_len ) && ( FALSE != going ) )
    {
        is_empty = bl_uart_buff_empty( CLI_UART );
        
        if ( FALSE == is_empty )
        {
            ch = bl_uart_getc( CLI_UART );
        
            switch ( ch )
            {
                case 0x0A:  /**< Line feed \n */
                case 0x0D:  /**< Carriage return \r */
                    going = FALSE;
                    
                    if ( i > 1 )
                    {
                        sl_memcpy ( cmd_buff[cmd_ind].line, buff, i );
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
                    bl_uart_send ( CLI_UART, (uint8_t*)"\r\n", 2 );
                    break;
                    
                case 0x09:  /**< Horizontal tab */
                case 0x08:  /**< Backspace */
                case 0x7F:  /**< DEL */
                    if ( ZERO != i )
                    {
                        i--;
                        bl_uart_send ( CLI_UART, (uint8_t*)"\b", 1 );                    
                    }
                    break;
                    
                case 0x41:
                    if ( ( 0x5B == buff[i-1] ) && ( 0x1B == buff[i-2] ) )
                    {
                        bl_uart_send ( CLI_UART, (uint8_t*)"\r> ", 3 );
                        
                        if ( cmd_delta < cmd_num )
                        {
                            cmd_delta++;
                        }
                        
                        ind = ( cmd_ind - cmd_delta ) % CLI_CMD_BUFF_NUM;
                        sl_memcpy ( buff
                                , cmd_buff[ind].line
                                , cmd_buff[ind].cmd_len
                                );
                        del = i - cmd_buff[ind].cmd_len - 2;
                        i = cmd_buff[ind].cmd_len;
                        bl_uart_send ( CLI_UART, buff, i );
                        
                        if ( del > 0 )
                        {
                            cli_delete_others ( del );
                        }
                    }
                    break;
                    
                case 0x42:
                    if ( ( 0x5B == buff[i-1] ) && ( 0x1B == buff[i-2] ) )
                    {
                        bl_uart_send ( CLI_UART, (uint8_t*)"\r> ", 3 );
                        
                        if ( cmd_delta > 0 )
                        {
                            cmd_delta--;
                        }
                        
                        ind = ( cmd_ind - cmd_delta ) % CLI_CMD_BUFF_NUM;
                        sl_memcpy ( buff
                                 , cmd_buff[ind].line
                                 , cmd_buff[ind].cmd_len
                                 );
                        del = i - cmd_buff[ind].cmd_len - 2;
                        i = cmd_buff[ind].cmd_len;
                        bl_uart_send ( CLI_UART, buff, i );
                        
                        if ( del > 0 )
                        {
                            cli_delete_others ( del );
                        }
                    }
                    break;
                
                default:
                    buff[i++] = ch;
                    
                    if ( ( 0x5B != ch ) && ( 0x1B != ch ) )
                    {
                        bl_uart_send ( CLI_UART, &ch, 1 );
                    }
            }
        }
    }
    
    return i;
}

static void cli_delete_others ( uint8_t num )
{
    uint8_t i;
    
    for ( i = 0 ; i < num; i++ )
    {
        bl_uart_send ( CLI_UART, (uint8_t*)" ", 1 );
    }
    
    for ( i = 0; i< num; i++ )
    {
        bl_uart_send ( CLI_UART, (uint8_t*)"\b", 1 );
    }
    
}

static void cli_fill_with_space ( uint8_t name_size )
{
    uint8_t space_size = 15;
    uint8_t i;
    uint8_t out_space[16];
    uint8_t out[SL_MAX_STRING_SIZE] = {0};
    
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
        strcat ( (char*)out_space, " " );
    }
    
    sl_sprintf_s ( out
                 , (uint8_t*)"%s"
                 , (uint8_t*)out_space
                 , sizeof ( out )
                 );
    
    bl_uart_send ( CLI_UART, out, sl_strnlen ( out, SL_MAX_STRING_SIZE ));
}

static void cli_parse_cmd ( Cli_Cmd_Args* args, uint8_t* str )
{
    char     delims[] = " ";
    char*    result = NULL;
    uint8_t  i = 0;
    uint8_t  out[SL_MAX_STRING_SIZE] = {0};
    
    result = strtok ( (char*)str, delims );
    
    while ( NULL != result )
    {
        if ( sl_strnlen ( (uint8_t*)result , CLI_CMD_MAX_ARG_SIZE ) >=
            CLI_CMD_MAX_ARG_SIZE )
        {
            sl_sprintf_s ( out
                         , (uint8_t*)"Error: Argument %s is too long!\r\n"
                         , (uint8_t*)result
                         , sizeof ( out )
                         );
            
            bl_uart_send ( CLI_UART
                           , out
                           , sl_strnlen ( out, SL_MAX_STRING_SIZE )
                           );
            
            break;
        }
        
        sl_strncpy( (uint8_t*)args->str[i]
                  , (uint8_t*)result
                  , CLI_CMD_MAX_ARG_SIZE
                  );
        
        if ( ( '0' == result[0] ) && ( 'x' == result[1] ) )
        {
            args->num[i] = strtoul ( result, NULL, 16 );
        }
        else
        {
            args->num[i] = sl_atoul ( (uint8_t*)result );
        }
        
        result = (char*) strtok ( NULL, delims );
        i++;
        
        if ( i >= CLI_CMD_MAX_ARG )
        {
            bl_uart_send ( CLI_UART
                         , (uint8_t*)"Error: "
                           "Maximal argument count reached!\r\n"
                         , 40
                         );
            break;
        }
    }
    
    args->count = i;
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
    
    if ( ZERO == args->count )
    {
        ret = CLI_RET_IGNORE_CMD;
    }
    
    if ( CLI_RET_IGNORE_CMD != ret )
    {
        if ( ZERO == sl_strncmp ( (uint8_t*)args->str[0]
                                , (uint8_t*)"help"
                                , CLI_CMD_MAX_LINE_SIZE ) ||
            ( ZERO == sl_strncmp ( (uint8_t*)args->str[0]
                                , (uint8_t*)"?"
                                , CLI_CMD_MAX_LINE_SIZE ) ) )
        {
            cli_print_help ();
            
            ret = CLI_RET_OK;
        }
    }
    
    if ( ( CLI_RET_OK != ret ) && ( CLI_RET_IGNORE_CMD != ret ) )
    {
        while ( i < table_size )
        {
            if ( ZERO == sl_strncmp ( (uint8_t*)args->str[0]
                                    , (uint8_t*)cli_cmd_table[i]->name
                                    , CLI_CMD_MAX_LINE_SIZE ) )
            {
                commands = (Cli_Cmd*)cli_cmd_table[i]->cmd_list;
                size     = cli_cmd_table[i]->list_size;
                
                if ( 1 == args->count )
                {
                    cli_print_help_sub_cmd ( (Cli_Cmd_List*)cli_cmd_table[i] );
                    ret = CLI_RET_OK;
                    break;
                }
                else
                {
                    if ( ZERO == sl_strncmp ( (uint8_t*)args->str[1]
                                            , (uint8_t*)"help"
                                            , CLI_CMD_MAX_LINE_SIZE ) ||
                       ( ZERO == sl_strncmp ( (uint8_t*)args->str[1]
                                            , (uint8_t*)"?"
                                            , CLI_CMD_MAX_LINE_SIZE ) ) )
                    {
                        cli_print_help_sub_cmd
                                ( (Cli_Cmd_List*)cli_cmd_table[i] );
                        ret = CLI_RET_OK;
                        break;
                    }
                    
                    for ( j = 0; j < size; j++ )
                    {
                        if ( ZERO == sl_strncmp ( (uint8_t*)args->str[1]
                                                , (uint8_t*)commands[j].name
                                                , CLI_CMD_MAX_LINE_SIZE ) )
                        {
                            ret = commands[j].cmd ( args );
                            break;
                        }
                        
                        if ( size - 1 == j )
                        {
                            cli_print_help_sub_cmd 
                                    ( (Cli_Cmd_List*)cli_cmd_table[i] );
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

static void cli_print_help_sub_cmd ( Cli_Cmd_List* subcmds )
{
    uint8_t size;
    uint8_t i;
    uint8_t out[SL_MAX_STRING_SIZE] = {0};
    
    size = subcmds->list_size;
    
    for ( i = 0; i < size; i++ )
    {
        sl_sprintf_s ( out
                     , (uint8_t*)"%s %s"
                     , (uint8_t*)subcmds->name
                     , sizeof ( out )
                     );
        sl_sprintf_s ( out
                     , out
                     , (uint8_t*)subcmds->cmd_list[i].name
                     , sizeof ( out )
                     );
        bl_uart_send ( CLI_UART
                     , out
                     , sl_strnlen ( out, SL_MAX_STRING_SIZE )
                     );

        cli_fill_with_space ( sl_strnlen
                            ( (uint8_t *) subcmds->name
                            , CLI_CMD_MAX_NAME_SIZE
                            )
                            + sl_strnlen
                            ( (uint8_t *) subcmds->cmd_list[i].name
                            , CLI_CMD_MAX_NAME_SIZE
                            )
                            + 1
                            );
        sl_sprintf_s ( out
                     , (uint8_t*)"%s\r\n"
                     , (uint8_t*)subcmds->cmd_list[i].description
                     , sizeof ( out )
                     );
        bl_uart_send ( CLI_UART
                     , out
                     , sl_strnlen ( out, SL_MAX_STRING_SIZE )
                     );       
    }       
}
