/**
  ******************************************************************************
  * @file    application/src/cli_esp8266.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    23-Mar-2019
  * @brief   ESP8266 module CLI commands
  ******************************************************************************
  */

#include "cli.h"

#include "esp8266.h"

#include <sl_string.h>

static Cli_Ret cli_wifi_info( Cli_Cmd_Args* args )
{
    Cli_Ret ret = CLI_RET_OK;

    (void) args;

    esp_dump_fw_info();

    return ret;
}

static Cli_Ret cli_wifi_ap( Cli_Cmd_Args* args )
{
    Cli_Ret ret = CLI_RET_OK;

    (void) args;

    esp_dump_available_ap();

    return ret;
}

static Cli_Ret cli_wifi_get_mac( Cli_Cmd_Args* args )
{
    Cli_Ret ret   = CLI_RET_OK;
    Esp_Ret e_ret = ESP_RET_NOT_AVAILABLE;
    uint8_t mac_buff[ESP_MAC_ADDR_SIZE] = {0};

    if (( args->num[2] != ESP_MODE_CLIENT ) 
     && ( args->num[2] != ESP_MODE_AP ))
    {
        bl_uart_send( UART_DBG, (uint8_t*) "Invalid mode!\r\n", 15 );
    }
    else
    {
        e_ret = esp_get_mac( mac_buff, args->num[2] );
    }

    if ( ESP_RET_OK != e_ret )
    {
        bl_uart_send( UART_DBG, (uint8_t*) "Error!\r\n", 8 );
        ret = CLI_RET_ERROR; 
    }
    else
    {
        if ( args->num[2] == ESP_MODE_CLIENT )
        {
            bl_uart_send( UART_DBG, (uint8_t*) "Station MAC address: ", 21 );
            bl_uart_send( UART_DBG, mac_buff, ESP_MAC_ADDR_SIZE );
            bl_uart_send( UART_DBG, (uint8_t*) "\r\n", 2 );
        }
        else
        {
            bl_uart_send( UART_DBG, (uint8_t*) "AP MAC address: ", 16 );
            bl_uart_send( UART_DBG, mac_buff, ESP_MAC_ADDR_SIZE );
            bl_uart_send( UART_DBG, (uint8_t*) "\r\n", 2 );
        }
    }

    return ret;
}

static Cli_Ret cli_wifi_set_mac( Cli_Cmd_Args* args )
{
    Cli_Ret ret   = CLI_RET_OK;
    Esp_Ret e_ret = ESP_RET_NOT_AVAILABLE;

    if (( args->num[2] != ESP_MODE_CLIENT ) 
     && ( args->num[2] != ESP_MODE_AP ))
    {
        bl_uart_send( UART_DBG, (uint8_t*) "Invalid mode!\r\n", 15 );
    }
    else
    {
        e_ret = esp_set_mac( args->str[3], args->num[2] );
    }

    if ( ESP_RET_OK != e_ret )
    {
        bl_uart_send( UART_DBG, (uint8_t*) "Error!\r\n", 8 );
        ret = CLI_RET_ERROR;
    }
    else
    {
        bl_uart_send( UART_DBG, (uint8_t*) "MAC address changed!\r\n", 22 );
    }

    return ret;
}

static Cli_Ret cli_wifi_get_err_log( Cli_Cmd_Args* args )
{
    Cli_Ret       ret = CLI_RET_OK;
    Esp_Err_Log_t error_cnt;
    uint8_t       out[SL_MAX_STRING_SIZE] = {0};

    (void) args;

    error_cnt = esp_get_error_cnt();

    sl_sprintf_d( out
                , (uint8_t*)"Error log info:\r\n"
                            "\tConnect to SSID: %d\r\n"
                            "\tSend/receive:    %d\r\n"
                            "\tTimeout:         %d\r\n"
                            "\tUnknown:         %d\r\n"
                , error_cnt.err_connect
                , SL_MAX_STRING_SIZE
                );

    sl_sprintf_d( out
                , out
                , error_cnt.err_packet
                , SL_MAX_STRING_SIZE
                );

    sl_sprintf_d( out
                , out
                , error_cnt.err_timeout
                , SL_MAX_STRING_SIZE
                );

    sl_sprintf_d( out
                , out
                , error_cnt.err_unknown
                , SL_MAX_STRING_SIZE
                );

    bl_uart_send( UART_DBG, out, sl_strnlen( out, SL_MAX_STRING_SIZE ));

    return ret;
}

static Cli_Ret cli_wifi_clear_err_log( Cli_Cmd_Args* args )
{
    Cli_Ret ret = CLI_RET_OK;

    (void) args;

    esp_reset_error_counter();

    return ret;
}

static const Cli_Cmd wifi_cmds[] = 
{
    { "fw_info"
    , cli_wifi_info
    , "Dump ESP8266 Firmware info"
    }
    ,
    { "get_mac"
    , cli_wifi_get_mac
    , "<mode> - 1 = Client, 2 = AP. Get ESP8266 MAC address"
    }
    ,
    { "set_mac"
    , cli_wifi_set_mac
    , "<mode> - 1 = Client, 2 = AP <mac>. Set ESP8266 MAC address"
    }
    ,
    { "get_log"
    , cli_wifi_get_err_log
    , "Dump error log"
    }
    ,
    { "cl_log"
    , cli_wifi_clear_err_log
    , "Clear error log"
    }
    ,
    { "ap_info"
    , cli_wifi_ap
    , "List available APs"
    }
};

const Cli_Cmd_List cmd_wifi_list = 
{ "wifi"
, wifi_cmds
, sizeof ( wifi_cmds ) / sizeof ( wifi_cmds[0] )
, "ESP8266 operations"
};
