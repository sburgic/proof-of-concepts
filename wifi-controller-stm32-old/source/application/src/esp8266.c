/**
  ******************************************************************************
  * @file    application/src/esp8266.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    25-Mar-2019
  * @brief   ESP8266 Wifi module operations
  ******************************************************************************
 */

#include "esp8266.h"

#include "bl_flash.h"

#include <sl_handle.h>
#include <sl_string.h>
#include <sl_time.h>
#include <sl_timeout.h>
#include <sl_mem.h>

#define ESP_TIMEOUT ((Sl_Time) 5000 )

typedef enum
{
    ESP_PAGE_IDX,
    ESP_PAGE_ERROR,
    ESP_PAGE_SUCCESS,
    ESP_PAGE_LOGIN_SENT
} Esp_Page_t;

typedef struct
{
    uint8_t     id;
    uint16_t    len;
    uint8_t     hdr_size;
    Esp_Page_t  req_page;
} Esp_Ipd;

typedef struct
{
    const uint8_t* html_name;
    const uint8_t  name_size;
    Esp_Page_t     page_type;
} Esp_Html_Idx_Req_t;

typedef struct
{
    uint8_t conn_id;
    bool_t  opened;
} Esp_Connection_t;

static Esp_t             esp_hdl;
static uint8_t           esp_buff[ESP_MAX_BUFF_SIZE];
static Esp_Connection_t  esp_connection[ESP_MAX_CONNECTIONS];
static uint32_t          esp_live_stats;
static uint8_t           esp_memo[ESP_MEMO_DATA_SIZE];
static uint8_t           esp_mac_addr[ESP_MAC_ADDR_SIZE];
static Esp_Relay_State_t esp_relay_state = ESP_RELAY_OFF;

static const Esp_Html_Idx_Req_t esp_idx_req[] =
{
    { (const uint8_t*) "GET / HTTP/1.1\r\n", 16, ESP_PAGE_IDX }
   ,{ (const uint8_t*) "GET /index.html? HTTP/1.1\r\n", 25, ESP_PAGE_IDX }
   ,{ (const uint8_t*) "GET /index.html HTTP/1.1\r\n", 24, ESP_PAGE_IDX }
   ,{ (const uint8_t*) "GET /index.htm HTTP/1.1\r\n", 23, ESP_PAGE_IDX }
   ,{ (const uint8_t*) "POST /index.html HTTP/1.1\r\n"
      , 24
      , ESP_PAGE_LOGIN_SENT
    }
};

static const uint16_t esp_idx_req_size = sizeof(esp_idx_req)
                                       / sizeof(esp_idx_req[0]);

static Esp_Ret esp_parse( uint8_t* cmd_reponse , uint16_t cmd_size );
static Esp_Ret esp_detect_http_rqv( Esp_Ipd* ipd );
static Esp_Ret esp_http_dump_html( Esp_Ipd ipd );
static Esp_Ret esp_open_tcp( void );
static void esp_pack_error_log( uint8_t* buff );

Esp_Ret esp_init( UART_Base base )
{
    Esp_Ret   ret  = ESP_RET_INV_HDL;
    HAL_Ret   hret;
    uint8_t   no_of_retries = 0;
    Esp_Cfg_t cfg_tmp;

    if ( HDL_IS_VALID( base ) )
    {
        esp_hdl.send      = bl_uart_send;
        esp_hdl.receive   = bl_uart_receive;
        esp_hdl.uart_data = bl_uart_get_handle( base );
        esp_hdl.base      = base;
        esp_hdl.cfg       = (Esp_Cfg_t*)ESP_CFG_DATA_ADDR;

        sl_memcpy( &cfg_tmp, esp_hdl.cfg, sizeof(Esp_Cfg_t));

        cfg_tmp.html_idx_page = (uint8_t*) ESP_CFG_HTML_IDX_ADDR;
        cfg_tmp.html_idx_size = ESP_CFG_HTML_IDX_SIZE;
        cfg_tmp.html_login_sent_page = (uint8_t*) ESP_CFG_HTML_LOGIN_SENT_ADDR;
        cfg_tmp.html_login_sent_size = ESP_CFG_HTML_LOGIN_SENT_SIZE;

        /* Initialize error counters only first time */
        if ( 0xFFFF == cfg_tmp.err.err_connect )
        {
            cfg_tmp.err.err_connect = 0;
        }
        if ( 0xFFFF == cfg_tmp.err.err_packet )
        {
            cfg_tmp.err.err_packet = 0;
        }
        if ( 0xFFFF == cfg_tmp.err.err_timeout )
        {
            cfg_tmp.err.err_timeout = 0;
        }
        if ( 0xFFFF == cfg_tmp.err.err_unknown )
        {
            cfg_tmp.err.err_unknown = 0;
        }

        hret = bl_flash_write((Esp_Cfg_t*) &cfg_tmp
                             , sizeof (Esp_Cfg_t)
                             , ESP_CFG_DATA_OFFSET
                             );

        if ( HAL_OK == hret )
        {
            /* Clear buffer on initialization because ESP8266 dumps info
             * on power-on.
             */
            sl_wait( 2000, SL_TIME_MSEC );
            bl_uart_buff_clear( base );

            do
            {
                /* Disable echo if not disabled by default */
                hret  = esp_hdl.send( base
                                    , ESP_ECHO_OFF
                                    , sl_strnlen( ESP_ECHO_OFF
                                                , SL_MAX_STRING_SIZE
                                                )
                                    );

                hret |= esp_hdl.send( base
                                    , ESP_AT
                                    , sl_strnlen( ESP_AT, SL_MAX_STRING_SIZE )
                                     );

                if ( HAL_OK == hret )
                {
                    ret = esp_parse( ESP_AT_OK, ESP_AT_OK_SIZE );
                }

                no_of_retries++;

            }
            while (( ESP_RET_OK != ret )
                  && ( no_of_retries < ESP_NO_OF_RETRIES ));
        }

        bl_uart_buff_clear( esp_hdl.base );
    }

    return ret;
}

Esp_Ret esp_set_mode( Esp_Mode mode )
{
    Esp_Ret   ret  = ESP_RET_INV_MODE;
    HAL_Ret   hret = HAL_ERROR;
    uint8_t   no_of_retries = 0;

    do
    {
        esp_hdl.send( esp_hdl.base
                    , ESP_RESET
                    , sl_strnlen( ESP_RESET
                                , SL_MAX_STRING_SIZE
                                )
                    );
        ret = esp_parse( ESP_AT_OK, ESP_AT_OK_SIZE );

        no_of_retries++;
    }
    while (( ESP_RET_OK != ret ) && ( no_of_retries < ESP_NO_OF_RETRIES ));

    no_of_retries = 0;

    do
    {
        switch ( mode )
        {
            case ESP_MODE_AP:
            esp_hdl.send( esp_hdl.base
                        , ESP_WIFI_MODE_AP
                        , sl_strnlen( ESP_WIFI_MODE_AP
                                    , SL_MAX_STRING_SIZE
                                    )
                        );
            ret = esp_parse( ESP_AT_OK, ESP_AT_OK_SIZE );
            break;

            case ESP_MODE_CLIENT:
            esp_hdl.send( esp_hdl.base
                        , ESP_WIFI_MODE_CL
                        , sl_strnlen( ESP_WIFI_MODE_CL
                                    , SL_MAX_STRING_SIZE
                                    )
                        );
            ret = esp_parse( ESP_AT_OK, ESP_AT_OK_SIZE );

            esp_hdl.send( esp_hdl.base
                        , ESP_WIFI_MUX_OFF
                        , sl_strnlen( ESP_WIFI_MUX_OFF, SL_MAX_STRING_SIZE )
                        );

            ret |= esp_parse( ESP_AT_OK, ESP_AT_OK_SIZE );
            break;

            default:
                break;
        }

        no_of_retries++;
    }
    while (( ESP_RET_OK != ret ) && ( no_of_retries < ESP_NO_OF_RETRIES ));

    if ( ESP_RET_OK == ret )
    {
        Esp_Cfg_t cfg_tmp;

        sl_memcpy( &cfg_tmp, esp_hdl.cfg, sizeof(Esp_Cfg_t));

        cfg_tmp.mode = ( ESP_MODE_AP == mode ) ? ESP_MODE_AP : ESP_MODE_CLIENT;

        /* Reset all error counters at mode change */
        esp_reset_error_counter();

        bl_uart_send( UART_DBG
                    , (uint8_t*)"Current ESP mode: "
                    , 18
                    );

        bl_uart_send( UART_DBG
                    , ( ESP_MODE_AP == mode ) ?
                                    (uint8_t*)"AP\r\n" : (uint8_t*)"Client\r\n"
                    , ( ESP_MODE_AP == mode ) ? 4 : 8
                    );

        hret = bl_flash_write((Esp_Cfg_t*)&cfg_tmp
                             , sizeof(Esp_Cfg_t)
                             , ESP_CFG_DATA_OFFSET
                            );

        if ( HAL_OK != hret )
        {
            ret = HAL_ERROR;
        }
    }

    bl_uart_buff_clear( esp_hdl.base );
    sl_wait( 200, SL_TIME_MSEC );

    return ret;
}

Esp_Mode esp_get_mode( void )
{
    return esp_hdl.cfg->mode;
}

Esp_Ret esp_dump_fw_info( void )
{
    Esp_Ret  ret  = ESP_RET_INV_HDL;
    HAL_Ret  hret;
    bool_t   is_timeout = FALSE;
    Sl_Time  timeout;

    hret = esp_hdl.send( esp_hdl.base
                       , ESP_VERSION
                       , sl_strnlen( ESP_VERSION, SL_MAX_STRING_SIZE )
                       );

    if ( HAL_OK == hret )
    {
        sl_set_timeout( ESP_TIMEOUT, SL_TIME_MSEC, &timeout );

        ret = ESP_RET_TIMED_OUT;

        do
        {
            is_timeout = sl_is_timeout( timeout );

            hret = esp_hdl.receive( esp_hdl.base
                                  , esp_buff
                                  , BL_UART_BUFF_SIZE
                                  , NULL
                                  );

            if ( HAL_OK == hret )
            {
                bl_uart_send( UART_DBG
                            , esp_buff
                            , sl_strnlen( esp_buff, ESP_MAX_BUFF_SIZE )
                            );

                ret = ESP_RET_OK;
            }
        }
        while ( FALSE == is_timeout );
    }

    bl_uart_buff_clear( esp_hdl.base );

    return ret;
}

Esp_Ret esp_dump_available_ap( void )
{
    Esp_Ret ret  = ESP_RET_INV_HDL;
    HAL_Ret hret;
    bool_t  is_timeout = FALSE;
    Sl_Time timeout;
    bool_t  done = FALSE;

    /* To list available APs the device must be in station mode */
    esp_hdl.send( esp_hdl.base
                , ESP_WIFI_MODE_CL
                , sl_strnlen( ESP_WIFI_MODE_CL
                            , SL_MAX_STRING_SIZE
                            )
                );
    ret  = esp_parse( ESP_AT_OK
                    , ESP_AT_OK_SIZE
                    );

    if ( ESP_RET_OK == ret )
    {
        bl_uart_buff_clear( esp_hdl.base );
        hret = esp_hdl.send( esp_hdl.base
                           , ESP_LIST_AVAILABLE_AP
                           , sl_strnlen( ESP_LIST_AVAILABLE_AP
                                       , SL_MAX_STRING_SIZE
                                       )
                           );

        if ( HAL_OK == hret )
        {
            sl_set_timeout( 2 * ESP_TIMEOUT, SL_TIME_MSEC, &timeout );

            ret = ESP_RET_TIMED_OUT;

            do
            {
                is_timeout = sl_is_timeout( timeout );

                hret = esp_hdl.receive( esp_hdl.base
                                      , esp_buff
                                      , ESP_MAX_BUFF_SIZE
                                      , NULL
                                      );

                if ( HAL_OK == hret )
                {
                    if ( 0 != sl_strncmp( esp_buff, ESP_AT_OK, ESP_AT_OK_SIZE ))
                    {
                        bl_uart_send( UART_DBG
                                    , esp_buff
                                    , sl_strnlen( esp_buff, ESP_MAX_BUFF_SIZE )
                                    );
                    }
                    else
                    {
                        done = TRUE;
                    }

                    ret = ESP_RET_OK;
                }
            }
            while (( FALSE == is_timeout ) && ( done != TRUE ));
        }
    }

    bl_uart_buff_clear( esp_hdl.base );

    return ret;
}

Esp_Ret esp_set_ssid( void )
{
    Esp_Ret ret = ESP_RET_TIMED_OUT;
    uint8_t mac_buff[ESP_MAC_ADDR_SIZE] = {0};

    ret = esp_get_mac( mac_buff, ESP_MODE_AP );

    if ( ESP_RET_OK == ret )
    {
        /* Set SSID name to MAC address of the SoftAP, no password */

        esp_hdl.send( esp_hdl.base
                    , (uint8_t*)"AT+CWSAP=\""
                    , 10
                    );

        esp_hdl.send( esp_hdl.base
                    , mac_buff
                    , ESP_MAC_ADDR_SIZE
                    );

        esp_hdl.send( esp_hdl.base
                    , (uint8_t*)"\",\"\",4,0\r\n"
                    , 10
                    );

        ret = esp_parse( ESP_AT_OK, ESP_AT_OK_SIZE );
    }

    if ( ESP_RET_OK == ret )
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"SSID set successfully.\r\n"
                    , 24
                    );
    }
    else
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"SSID set failed.\r\n"
                    , 18
                    );
    }

    bl_uart_buff_clear( esp_hdl.base );
    sl_wait( 200, SL_TIME_MSEC );

    return ret;
}

Esp_Ret esp_enable_dhcp( void )
{
    Esp_Ret ret = ESP_RET_TIMED_OUT;
    uint8_t no_of_retries = 0;

    do
    {
        esp_hdl.send( esp_hdl.base
                    , ESP_WIFI_SET_IP
                    , sl_strnlen ( ESP_WIFI_SET_IP, SL_MAX_STRING_SIZE )
                    );

        ret = esp_parse( ESP_AT_OK, ESP_AT_OK_SIZE );

        no_of_retries++;
    }
    while (( ESP_RET_OK != ret ) && ( no_of_retries < ESP_NO_OF_RETRIES ));

    if ( ESP_RET_OK == ret )
    {
        do
        {
            esp_hdl.send( esp_hdl.base
                        , ESP_WIFI_DHCP_ON
                        , sl_strnlen( ESP_WIFI_DHCP_ON, SL_MAX_STRING_SIZE )
                        );

            ret = esp_parse ( ESP_AT_OK, ESP_AT_OK_SIZE  );

            no_of_retries++;
        }
        while (( ESP_RET_OK != ret ) && ( no_of_retries < ESP_NO_OF_RETRIES ));
    }

    if ( ESP_RET_OK == ret )
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"DHCP enabled.\r\n"
                    , 15
                    );

        bl_uart_send( UART_DBG
                    , (uint8_t*)"Gateway IP address: 192.168.1.1\r\n"
                    , 33
                    );
    }
    else
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"DHCP failed.\r\n"
                    , 14
                    );
    }

    bl_uart_buff_clear( esp_hdl.base );
    sl_wait( 200, SL_TIME_MSEC );

    return ret;
}

Esp_Ret esp_create_tcp_server( void )
{
    Esp_Ret ret = ESP_RET_TIMED_OUT;
    uint8_t no_of_retries = 0;

    do
    {
        esp_hdl.send( esp_hdl.base
                    , ESP_WIFI_MUX_ON
                    , sl_strnlen( ESP_WIFI_MUX_ON, SL_MAX_STRING_SIZE )
                    );

        ret = esp_parse( ESP_AT_OK, ESP_AT_OK_SIZE );

        no_of_retries++;
    }
    while (( ESP_RET_OK != ret ) && ( no_of_retries < ESP_NO_OF_RETRIES ));

    if ( ESP_RET_OK == ret )
    {
        no_of_retries = 0;

        do
        {
            esp_hdl.send( esp_hdl.base
                        , ESP_WIFI_TCP_SERVER
                        , sl_strnlen( ESP_WIFI_TCP_SERVER
                                    , SL_MAX_STRING_SIZE
                                    )
                        );

            ret = esp_parse( ESP_AT_OK, ESP_AT_OK_SIZE );

            no_of_retries++;
        }
        while (( ESP_RET_OK != ret ) && ( no_of_retries < ESP_NO_OF_RETRIES ));
    }

    if ( ESP_RET_OK == ret )
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"TCP server started.\r\n"
                    , 21
                    );

        bl_uart_send( UART_DBG
                    , (uint8_t*)"Listening on port 80.\r\n"
                    , 23
                    );
    }
    else
    {
        bl_uart_send( UART_DBG
                    , (uint8_t*)"TCP server failed.\r\n"
                    , 20
                    );
    }

    bl_uart_buff_clear( esp_hdl.base );
    sl_wait( 200, SL_TIME_MSEC );

    return ret;
}

Esp_Ret esp_tcp_listen( void )
{
    Esp_Ret ret = ESP_RET_NOT_AVAILABLE;
    Esp_Ipd ipd = {0};

    while ( ESP_RET_OK != ret )
    {
        ret = esp_detect_http_rqv( &ipd );
    }

    if ( ESP_RET_OK == ret )
    {
        ret = esp_http_dump_html( ipd );
    }

    return ret;
}

static Esp_Ret esp_parse( uint8_t* cmd_response
                        , uint16_t cmd_size
                        )
{
    Esp_Ret  ret  = ESP_RET_NOT_AVAILABLE;
    HAL_Ret  hret = HAL_ERROR;
    int16_t  res;
    bool_t   is_timeout = FALSE;
    bool_t   is_empty   = FALSE;
    Sl_Time  timeout;
    uint16_t read_bytes = 0;

    sl_set_timeout( ESP_TIMEOUT, SL_TIME_MSEC, &timeout );
    // sl_wait( 1, SL_TIME_SEC ); /* Wait for the data to be received */

    do
    {
        is_timeout = sl_is_timeout( timeout );
        is_empty   = bl_uart_buff_empty( esp_hdl.base );

        if (( FALSE == is_timeout) && ( FALSE == is_empty ))
        {
            hret = esp_hdl.receive( esp_hdl.base
                                , esp_buff
                                , BL_UART_BUFF_SIZE
                                , &read_bytes
                                );
        }

        if (( HAL_OK == hret ) && ( 0 != read_bytes ))
        {
            res = sl_strncmp( &esp_buff[read_bytes - cmd_size]
                            , cmd_response
                            , cmd_size
                            );

            if ( 0 != res )
            {
                ret = ESP_RET_NOT_AVAILABLE;
            }
            else
            {
                ret = ESP_RET_OK;
            }
        }
    }
    while (( FALSE == is_timeout ) && ( ESP_RET_OK != ret ));

    if ( FALSE != is_timeout )
    {
        ret = ESP_RET_TIMED_OUT;
    }

    return ret;
}

static Esp_Ret esp_receive_mac( void )
{
    Esp_Ret  ret  = ESP_RET_NOT_AVAILABLE;
    HAL_Ret  hret = HAL_ERROR;
    int16_t  res;
    bool_t   is_timeout = FALSE;
    bool_t   is_empty   = FALSE;
    Sl_Time  timeout;
    uint16_t read_bytes = 0;

    sl_set_timeout( ESP_TIMEOUT, SL_TIME_MSEC, &timeout );

    do
    {
        is_timeout = sl_is_timeout( timeout );
        is_empty   = bl_uart_buff_empty( esp_hdl.base );

        if (( FALSE == is_timeout) && ( FALSE == is_empty ))
        {
            hret = esp_hdl.receive( esp_hdl.base
                                  , esp_buff
                                  , BL_UART_BUFF_SIZE
                                  , &read_bytes
                                  );
        }

        if (( HAL_OK == hret ) && ( 0 != read_bytes ))
        {
            res = sl_strncmp( &esp_buff[read_bytes - ESP_MAC_ADDR_REPLY_SIZE]
                            , ESP_WIFI_MAC_TAG_AP
                            , ESP_WIFI_MAC_TAG_AP_SIZE
                            );

                            // uint8_t out[64] = {0};
                            // sl_sprintf_d( out, "%d\r\n", read_bytes, 5 );
                            // bl_uart_send(UART_DBG, out, sl_strnlen(out, 64));

            if ( 0 != res )
            {
                ret = ESP_RET_NOT_AVAILABLE;
            }
            else
            {
                ret = ESP_RET_OK;
            }
        }
    }
    while (( FALSE == is_timeout ) && ( ESP_RET_OK != ret ));

    if ( FALSE != is_timeout )
    {
        ret = ESP_RET_TIMED_OUT;
    }

    return ret;
}

static Esp_Ret esp_detect_http_rqv( Esp_Ipd* ipd )
{
    Esp_Ret  ret;
    uint8_t  temp[ESP_HTTP_LEN_SIZE] = {0};
    uint8_t  i = 0;
    uint16_t j = 0;
    int16_t  res;

    ret = esp_parse( ESP_HTTP_IPD_START, ESP_HTTP_IPD_START_SIZE );

    if ( ESP_RET_OK == ret )
    {

        /* Assume ID is only 1 digit and convert to integer */
        ipd->id = esp_buff[ESP_HTTP_IPD_START_SIZE] - 48;

        while ( i < 4 )
        {
            temp[i] = esp_buff[ ESP_HTTP_LEN_START + i ];

            /* After length we expect ':' */
            if ( esp_buff[ ESP_HTTP_LEN_START + i ] == ':' )
            {
                temp[i] = 0;
                break;
            }

            i++;
        }

        ipd->len = (uint16_t) sl_atoul( temp );

        if ( 0 == ipd->len )
        {
            ret = ESP_RET_NOT_AVAILABLE;
        }
    }

    if ( ESP_RET_OK == ret )
    {
        do
        {
            res = sl_strncmp( &esp_buff[ESP_HTTP_LEN_START + i + 1]
                            , esp_idx_req[j].html_name
                            , esp_idx_req[j].name_size
                            );
            j++;
        } while (( 0 != res ) && ( j < esp_idx_req_size ));

        if ( 0 != res )
        {
            ipd->req_page = ESP_PAGE_ERROR;
            ipd->hdr_size = 0;

            sl_sprintf_d( esp_buff
                        , ESP_WIFI_CIPCLOSE
                        , ipd->id
                        , ESP_MAX_BUFF_SIZE
                        );

            esp_hdl.send( esp_hdl.base
                        , esp_buff
                        , sl_strnlen( esp_buff, ESP_MAX_BUFF_SIZE )
                        );

            esp_connection[ipd->id].opened = FALSE;
        }
        else
        {
            ipd->req_page = esp_idx_req[j-1].page_type;
            ipd->hdr_size = ESP_HTTP_IPD_START_SIZE + ESP_HTTP_ID_SIZE
                          + esp_idx_req[j-1].name_size - 1 + i;
        }
    }

    bl_uart_buff_clear( esp_hdl.base );

    return ret;
}

static Esp_Ret esp_http_dump_html( Esp_Ipd ipd )
{
    Esp_Ret  ret  = ESP_RET_OK;
    uint32_t size = 0;
    uint8_t* page = NULL;
    uint8_t  ch = 0;
    bool_t   is_timeout = FALSE;
    Sl_Time  timeout;
    uint8_t  no_of_retries = 0;

    switch ( ipd.req_page )
    {
        case ESP_PAGE_IDX:
        page = esp_hdl.cfg->html_idx_page;
        size = esp_hdl.cfg->html_idx_size;
        break;

        case ESP_PAGE_LOGIN_SENT:
        page = esp_hdl.cfg->html_login_sent_page;
        size = esp_hdl.cfg->html_login_sent_size;
        break;

        default:
        ret = ESP_RET_NOT_AVAILABLE;
        break;
    }

    if ( ESP_RET_OK == ret )
    {

        sl_sprintf_d( esp_buff
                    , ESP_HTTP_CIP_SEND
                    , ipd.id
                    , SL_MAX_STRING_SIZE
                    );

        sl_sprintf_d( esp_buff
                    , esp_buff
                    , size
                    , ESP_MAX_BUFF_SIZE
                    );

        esp_hdl.send( esp_hdl.base
                    , esp_buff
                    , sl_strnlen( esp_buff, ESP_MAX_BUFF_SIZE )
                    );

        sl_set_timeout( ESP_TIMEOUT, SL_TIME_MSEC, &timeout );

        do
        {
            is_timeout = sl_is_timeout( timeout );
            ch = bl_uart_getc( esp_hdl.base );
        }
        while (( ch != '>' ) && ( FALSE == is_timeout ));

        if ( FALSE != is_timeout )
        {
            ret = ESP_RET_TIMED_OUT;
        }
    }

    if ( ESP_RET_OK == ret )
    {
        esp_hdl.send( esp_hdl.base
                    , page
                    , size
                    );

        /* Try several times because first time browser is attempting to
         * get the favicon and several HTTP requests are received. Due to that
         * the UART buffer is overwritten and SEND_OK could be missed.
         */
        do
        {
            ret = esp_parse( ESP_SEND_OK, ESP_SEND_OK_SIZE );
            no_of_retries++;
        }
        while (( ESP_RET_OK != ret ) && ( no_of_retries < ESP_NO_OF_RETRIES ));

        /* See Espressif AT command set - AT+CIPSEND.
         * We shall wait at least 1 second after AT+CIPSEND before issuing
         * another command.
         */
        sl_wait( 1, SL_TIME_SEC );

        if ( ESP_RET_OK == ret )
        {
            sl_sprintf_d( esp_buff
                        , ESP_WIFI_CIPCLOSE
                        , ipd.id
                        , ESP_MAX_BUFF_SIZE
                        );

            esp_hdl.send( esp_hdl.base
                        , esp_buff
                        , sl_strnlen(esp_buff, ESP_MAX_BUFF_SIZE)
                        );

            esp_connection[ipd.id].opened = FALSE;

            if ( ESP_PAGE_LOGIN_SENT == ipd.req_page )
            {
                ret = esp_load_cfg();
            }
        }
    }

    bl_uart_buff_clear( esp_hdl.base );

    return ret;
}

static void esp_pack_error_log( uint8_t* buff )
{
    sl_sprintf_d( buff
                , (uint8_t*)"%d.%d.%d.%d"
                , esp_hdl.cfg->err.err_connect
                , ESP_MEMO_DATA_SIZE
                );

    sl_sprintf_d( buff
                , buff
                , esp_hdl.cfg->err.err_packet
                , ESP_MEMO_DATA_SIZE
                );

    sl_sprintf_d( buff
                , buff
                , esp_hdl.cfg->err.err_timeout
                , ESP_MEMO_DATA_SIZE
                );

    sl_sprintf_d( buff
                , buff
                , esp_hdl.cfg->err.err_unknown
                , ESP_MEMO_DATA_SIZE
                );
}

static Esp_Ret esp_open_tcp( void )
{
    Esp_Ret ret;
    uint8_t i = 0;

    do
    {
        esp_hdl.send( esp_hdl.base
                    , ESP_OPEN_TCP_TO_SERVER
                    , sl_strnlen( ESP_OPEN_TCP_TO_SERVER
                                , SL_MAX_STRING_SIZE
                                )
                    );

        ret = esp_parse( ESP_AT_OK, ESP_AT_OK_SIZE );
        i++;
    } while (( i < ESP_NO_OF_RETRIES ) && ( ESP_RET_OK != ret ));

    return ret;
}

Esp_Ret esp_load_cfg( void )
{
    Esp_Ret   ret  = ESP_RET_OK;
    HAL_Ret   hret = HAL_ERROR;
    uint8_t*  found = NULL;
    Esp_Cfg_t cfg_tmp;
    uint8_t   i = 0;

    sl_memcpy( &cfg_tmp, esp_hdl.cfg, sizeof(Esp_Cfg_t));

    sl_wait( 1, SL_TIME_SEC );

    found = sl_strstr( esp_hdl.uart_data->buff
                      , ESP_HTTP_NETWORK_TAG
                      );

    if ( NULL != found )
    {
        found += ESP_HTTP_NETWORK_TAG_SIZE;

        while ( found[i] != '\r')
        {
            cfg_tmp.ssid[i] = found[i];
            i++;
        }
        while ( i < ESP_WIFI_SSID_SIZE )
        {
            cfg_tmp.ssid[i] = 0;
            i++;
        }
    }
    else
    {
        ret  = ESP_RET_NOT_AVAILABLE;
    }

    if ( ESP_RET_OK == ret )
    {
        found = sl_strstr( esp_hdl.uart_data->buff
                         , ESP_HTTP_PASSWORD_TAG
                         );

        if ( NULL != found )
        {
            i = 0;
            found += ESP_HTTP_PASSWORD_TAG_SIZE;

            while ( found[i] != '\r')
            {
                cfg_tmp.password[i] = found[i];
                i++;
            }
            while ( i < ESP_WIFI_PASS_SIZE )
            {
                cfg_tmp.password[i] = 0;
                i++;
            }
        }
        else
        {
            ret  = ESP_RET_NOT_AVAILABLE;
        }
    }

    if ( ESP_RET_OK == ret )
    {
        cfg_tmp.mode = ESP_MODE_CLIENT;

        hret = bl_flash_write((Esp_Cfg_t*) &cfg_tmp
                             , sizeof (Esp_Cfg_t)
                             , ESP_CFG_DATA_OFFSET
                             );

        bl_uart_send( UART_DBG, (uint8_t*)"Login data sent.\r\n", 18 );
        bl_uart_send( UART_DBG, (uint8_t*)"SSID: ", 6 );
        bl_uart_send( UART_DBG
                    , cfg_tmp.ssid
                    , sl_strnlen( cfg_tmp.ssid, ESP_WIFI_SSID_SIZE )
                    );
        bl_uart_send( UART_DBG, (uint8_t*)"\r\n", 2 );

        bl_uart_send( UART_DBG, (uint8_t*)"Password: ", 10 );
        bl_uart_send( UART_DBG
                    , cfg_tmp.password
                    , sl_strnlen( cfg_tmp.password, ESP_WIFI_PASS_SIZE )
                    );
        bl_uart_send( UART_DBG, (uint8_t*)"\r\n\r\n", 4 );

        esp_hdl.send( esp_hdl.base
                    , ESP_RESET
                    , sl_strnlen( ESP_RESET
                                , SL_MAX_STRING_SIZE
                                )
                    );

        /* Wait is required to dump all of the content to the console */
        sl_wait( 500, SL_TIME_MSEC );
    }

    if ( HAL_OK == hret )
    {
        NVIC_SystemReset();
    }
    else
    {
        ret = ESP_RET_NOT_AVAILABLE;
    }

    return ret;
}

Esp_Ret esp_detect_incoming( uint8_t* connection_id )
{
    Esp_Ret ret   = ESP_RET_NOT_AVAILABLE;
    uint8_t ch[2] = {0};

    do
    {
        ch[0] = bl_uart_getc( esp_hdl.base );

        if (( '0' == ch[0] ) || ( '1' == ch[0] )
         || ( '2' == ch[0] ) || ( '3' == ch[0] ))
        {
            ch[1] = bl_uart_getc( esp_hdl.base );

            if ( ',' == ch[1] )
            {
                *connection_id = ch[0] - 48;

                ret = ESP_RET_OK;
            }
        }

    } while ( ESP_RET_OK != ret );

    return ret;
}

Esp_Ret esp_get_mac( uint8_t* buff, Esp_Mode mode )
{
    Esp_Ret ret  = ESP_RET_INV_HDL;
    uint8_t no_of_retries = 0;

    bl_uart_buff_clear( esp_hdl.base );

    if ( 0 != buff )
    {
        ret = ESP_RET_NOT_AVAILABLE;

        do
        {
            /* Assume the mode is always valid */
            if ( ESP_MODE_CLIENT != mode )
            {
                esp_hdl.send( esp_hdl.base
                            , ESP_WIFI_GET_MAC_AP
                            , sl_strnlen( ESP_WIFI_GET_MAC_AP
                                        , SL_MAX_STRING_SIZE
                                        )
                            );

                ret  = esp_receive_mac();
            }
            else
            {
                esp_hdl.send( esp_hdl.base
                            , ESP_WIFI_GET_MAC_ST
                            , sl_strnlen( ESP_WIFI_GET_MAC_ST
                                        , SL_MAX_STRING_SIZE
                                        )
                            );

                ret  = esp_parse( ESP_WIFI_MAC_TAG_ST
                                , ESP_MAC_ADDR_REPLY_SIZE
                                );
            }

            no_of_retries++;
        }
        while (( ESP_RET_OK != ret ) && ( no_of_retries < ESP_NO_OF_RETRIES ));

        if ( ESP_RET_OK == ret )
        {
            sl_memcpy( buff
                     , &esp_buff[ESP_MODE_AP == mode ? \
                       ESP_WIFI_MAC_TAG_AP_SIZE : ESP_WIFI_MAC_TAG_ST_SIZE ]
                     , ESP_MAC_ADDR_SIZE
                     );
        }
    }

    sl_wait( 1, SL_TIME_SEC );

    return ret;
}

Esp_Ret esp_set_mac( uint8_t* buff, Esp_Mode mode )
{
    Esp_Ret ret  = ESP_RET_INV_HDL;
    uint8_t no_of_retries = 0;

    if ( 0 != buff )
    {
        ret = ESP_RET_NOT_AVAILABLE;

        do
        {
            /* Assume the mode is always valid */
            if ( ESP_MODE_CLIENT != mode )
            {
                esp_hdl.send( esp_hdl.base
                            , ESP_WIFI_SET_MAC_AP
                            , sl_strnlen( ESP_WIFI_SET_MAC_AP
                                        , SL_MAX_STRING_SIZE
                                        )
                            );
            }
            else
            {
                /* To change the MAC address mode must be station */
                esp_hdl.send( esp_hdl.base
                            , ESP_WIFI_MODE_CL
                            , sl_strnlen( ESP_WIFI_MODE_CL
                                        , SL_MAX_STRING_SIZE
                                        )
                            );
                ret  = esp_parse( ESP_AT_OK
                                , ESP_AT_OK_SIZE
                                );
                if ( ESP_RET_OK == ret )
                {
                    esp_hdl.send( esp_hdl.base
                                , ESP_WIFI_SET_MAC_ST
                                , sl_strnlen( ESP_WIFI_SET_MAC_ST
                                            , SL_MAX_STRING_SIZE
                                            )
                                );
                }
            }

            esp_hdl.send( esp_hdl.base
                        , buff
                        , ESP_MAC_ADDR_SIZE
                        );

            esp_hdl.send( esp_hdl.base
                        , (uint8_t*)"\"\r\n"
                        , 3
                        );

            ret  = esp_parse( ESP_AT_OK
                            , ESP_AT_OK_SIZE
                            );

            no_of_retries++;
        }
        while (( ESP_RET_OK != ret ) && ( no_of_retries < ESP_NO_OF_RETRIES ));
    }

    return ret;
}

Esp_Ret esp_close_all_connections( void )
{
    Esp_Ret ret = ESP_RET_OK;
    uint8_t i;

    for ( i = 0; i < ESP_MAX_CONNECTIONS; i++ )
    {
        if ( FALSE != esp_connection[i].opened )
        {
            sl_sprintf_d( esp_buff
                        , ESP_WIFI_CIPCLOSE
                        , i
                        , ESP_MAX_BUFF_SIZE
                        );

            esp_hdl.send( esp_hdl.base
                        , esp_buff
                        , sl_strnlen( esp_buff, ESP_MAX_BUFF_SIZE )
                        );

            esp_connection[i].opened = FALSE;
        }
    }

    return ret;
}

Esp_Ret esp_connect_to_ap( uint8_t* ssid, uint8_t* password )
{
    Esp_Ret ret = ESP_RET_OK;
    uint8_t i;
    int16_t len;
    uint8_t escape_char = 0x5C;

    bl_uart_buff_clear( esp_hdl.base );

    /* If no credendials provided read ssid and password from config */
    if (( 0 == ssid ) && ( 0 == password ))
    {
        ssid     = esp_hdl.cfg->ssid;
        password = esp_hdl.cfg->password;
    }

    len = sl_strnlen( ssid, ESP_WIFI_SSID_SIZE );

    if ( len < 1 )
    {
        ret = ESP_RET_SSID_FAILED;
    }
    else
    {
        esp_hdl.send( esp_hdl.base
                    , ESP_WIFI_CONNECT
                    , sl_strnlen( ESP_WIFI_CONNECT, ESP_MAX_BUFF_SIZE )
                    );

        for ( i = 0; (( i < len ) && ( 0 != ssid[i] )); i++ )
        {
            if (( 0x22 == ssid[i] ) || ( 0x2C == ssid[i] )
            || (( 0x5C == ssid[i] )))
            {
                esp_hdl.send( esp_hdl.base
                            , &escape_char
                            , 1
                            );
            }
            esp_hdl.send( esp_hdl.base
                        , &ssid[i]
                        , 1
                        );
        }

        esp_hdl.send( esp_hdl.base
                    , (uint8_t*) "\",\""
                    , 3
                    );

        len = sl_strnlen( password, ESP_WIFI_PASS_SIZE );

        for ( i = 0; (( i < len ) && ( 0 != password[i] )); i++ )
        {
            if (( 0x22 == password[i] ) || ( 0x2C == password[i] )
            || (( 0x5C == password[i] )))
            {
                esp_hdl.send( esp_hdl.base
                            , &escape_char
                            , 1
                            );
            }
            esp_hdl.send( esp_hdl.base
                        , &password[i]
                        , 1
                        );
        }
        esp_hdl.send( esp_hdl.base
                    , (uint8_t*) "\"\r\n"
                    , 3
                    );
    }

    if ( ESP_RET_OK == ret )
    {
        i = 0;
    
        do
        {
            ret = esp_parse( ESP_WIFI_CON, ESP_WIFI_CON_SIZE );
            i++;
        } while (( i < ESP_NO_OF_RETRIES ) && ( ESP_RET_OK != ret ));
    }

    if ( ESP_RET_OK == ret )
    {
        i = 0;

        do
        {
            ret = esp_parse( ESP_WIFI_GOT_IP, ESP_WIFI_GOT_IP_SIZE );
            i++;
        } while (( i < ESP_NO_OF_RETRIES ) && ( ESP_RET_OK != ret ));
    }
    else
    {
        ret = ESP_RET_SSID_FAILED;
    }

    return ret;
}

Esp_Ret esp_send_temp( uint8_t* temp )
{
    Esp_Ret  ret;
    uint8_t  ch  = 0;
    uint8_t  i   = 0;
    bool_t   is_timeout = FALSE;
    Sl_Time  timeout;
    uint8_t* found = NULL;
    uint8_t  rly_state[ESP_HTTP_RELAY_ST_MAX_SIZE] = {0};

    ret = esp_get_mac( esp_mac_addr, ESP_MODE_CLIENT );
    esp_pack_error_log( esp_memo );

    if (( 0 != temp ) && ( ESP_RET_OK == ret ))
    {
        ret = esp_open_tcp();

        if ( ret == ESP_RET_OK )
        {
            sl_wait( 1, SL_TIME_SEC );
            bl_uart_buff_clear( esp_hdl.base );

#if ( ESP_HTTP_TYPE_POST == 1 )
            sl_sprintf_d( esp_buff
                        , ESP_HTTP_CIP_SEND_L
                        , sl_strnlen( ESP_POST_BEGIN, SL_MAX_STRING_SIZE )
                        + ESP_MAC_ADDR_SIZE
                        + sl_strnlen( ESP_TEMP_BEGIN, SL_MAX_STRING_SIZE )
                        + sl_strnlen( temp, SL_MAX_STRING_SIZE )
                        + sl_strnlen( ESP_MEMO_BEGIN, SL_MAX_STRING_SIZE )
                        + sl_strnlen( esp_memo, ESP_MEMO_DATA_SIZE )
                        + sl_strnlen( ESP_POST_FINISH, SL_MAX_STRING_SIZE )
                        , ESP_MAX_BUFF_SIZE
                        );
#else
            sl_sprintf_d( esp_buff
                        , ESP_HTTP_CIP_SEND_L
                        , sl_strnlen( ESP_GET_BEGIN, SL_MAX_STRING_SIZE )
                        + ESP_MAC_ADDR_SIZE
                        + sl_strnlen( ESP_TEMP_BEGIN, SL_MAX_STRING_SIZE )
                        + sl_strnlen( temp, SL_MAX_STRING_SIZE )
                        + sl_strnlen( ESP_MEMO_BEGIN, SL_MAX_STRING_SIZE )
                        + sl_strnlen( esp_memo, ESP_MEMO_DATA_SIZE )
                        + sl_strnlen( ESP_GET_FINISH, SL_MAX_STRING_SIZE )
                        , ESP_MAX_BUFF_SIZE
                        );
#endif

            esp_hdl.send( esp_hdl.base
                        , esp_buff
                        , sl_strnlen( esp_buff, ESP_MAX_BUFF_SIZE )
                        );

            sl_set_timeout( ESP_TIMEOUT, SL_TIME_MSEC, &timeout );

            do
            {
                is_timeout = sl_is_timeout( timeout );
                ch = bl_uart_getc( esp_hdl.base );
            }
            while (( ch != '>' ) && ( FALSE == is_timeout ));

            if ( FALSE == is_timeout )
            {
                sl_wait( 1, SL_TIME_SEC );
                bl_uart_buff_clear( esp_hdl.base );

#if ( ESP_HTTP_TYPE_POST == 1 )
                esp_hdl.send( esp_hdl.base
                            , ESP_POST_BEGIN
                            , sl_strnlen( ESP_POST_BEGIN, SL_MAX_STRING_SIZE )
                            );
#else
                esp_hdl.send( esp_hdl.base
                            , ESP_GET_BEGIN
                            , sl_strnlen( ESP_GET_BEGIN, SL_MAX_STRING_SIZE )
                            );
#endif

                esp_hdl.send( esp_hdl.base
                            , esp_mac_addr
                            , ESP_MAC_ADDR_SIZE
                            );

                esp_hdl.send( esp_hdl.base
                            , ESP_TEMP_BEGIN
                            , sl_strnlen( ESP_TEMP_BEGIN, SL_MAX_STRING_SIZE )
                            );

                esp_hdl.send( esp_hdl.base
                            , temp
                            , sl_strnlen( temp, SL_MAX_STRING_SIZE )
                            );

                esp_hdl.send( esp_hdl.base
                            , ESP_MEMO_BEGIN
                            , sl_strnlen( ESP_MEMO_BEGIN, SL_MAX_STRING_SIZE )
                            );

                esp_hdl.send( esp_hdl.base
                            , esp_memo
                            , sl_strnlen( esp_memo, ESP_MEMO_DATA_SIZE )
                            );

#if ( ESP_HTTP_TYPE_POST == 1 )
                esp_hdl.send( esp_hdl.base
                            , ESP_POST_FINISH
                            , sl_strnlen( ESP_POST_FINISH, SL_MAX_STRING_SIZE )
                            );
#else
                esp_hdl.send( esp_hdl.base
                            , ESP_GET_FINISH
                            , sl_strnlen( ESP_GET_FINISH, SL_MAX_STRING_SIZE )
                            );
#endif

                /* Wait 1 second after issuing the AT+CIPSEND */
                sl_wait( 1, SL_TIME_SEC );

                ret = esp_parse( ESP_SEND_OK, ESP_SEND_OK_SIZE );

                /* Detect server response before checking
                 * the connection status
                 */
                if ( ESP_RET_OK == ret )
                {
                    ret = esp_parse( ESP_HTTP_IPD_START
                                   , ESP_HTTP_IPD_START_SIZE
                                   );
                }

                if ( ESP_RET_OK == ret )
                {
                    sl_wait( 1, SL_TIME_SEC );

                    found = sl_strstr( esp_hdl.uart_data->buff
                                     , ESP_HTTP_RELAY_TAG
                                     );

                    if ( NULL != found )
                    {
                        found += ESP_HTTP_RELAY_TAG_SIZE;
                        i = 0;

                        while ( found[i] != '!' )
                        {
                            rly_state[i] = found[i];
                            i++;
                        }

                        if ( 0 == sl_strncmp( rly_state, (uint8_t*)"on", 2 ))
                        {
                            esp_relay_set_state( ESP_RELAY_ON );
                        }
                        else if ( 0 == \
                                sl_strncmp( rly_state, (uint8_t*)"off", 3 ))
                        {
                            esp_relay_set_state( ESP_RELAY_OFF );
                        }
                        else
                        {
                            ret = ESP_RET_INV_MODE;
                        }
                    }
                }

                if ( ESP_RET_OK == ret )
                {
                    esp_hdl.send( esp_hdl.base
                                , ESP_WIFI_STATUS
                                , sl_strnlen( ESP_WIFI_STATUS
                                            , SL_MAX_STRING_SIZE
                                            )
                                );
                }

                ret = esp_parse( ESP_WIFI_ST_TAG, ESP_WIFI_ST_TAG_SZ );

                if (( ESP_RET_OK == ret )
                && ( esp_buff[ESP_WIFI_ST_TAG_SZ] != ESP_WIFI_ST_DISCONNECTED ))
                {
                    esp_hdl.send( esp_hdl.base
                                , (uint8_t*)"AT+CIPCLOSE\r\n"
                                , 13
                                );
                }
            }
            else
            {
                ret = ESP_RET_TIMED_OUT;
            }
        }
        else
        {
            ret = ESP_RET_NO_CONNECTION;
        }
    }

    return ret;
}

Esp_Ret esp_send_ack( void )
{
    Esp_Ret ret;
    uint8_t ch  = 0;
    bool_t  is_timeout = FALSE;
    Sl_Time timeout;

    ret = esp_get_mac( esp_mac_addr, ESP_MODE_CLIENT );

    if ( ESP_RET_OK == ret )
    {
        ret = esp_open_tcp();
    }

    if ( ESP_RET_OK == ret )
    {
        sl_wait( 1, SL_TIME_SEC );
        bl_uart_buff_clear( esp_hdl.base );

        sl_sprintf_d( esp_buff
                    , ESP_HTTP_CIP_SEND_L
                    , sl_strnlen( ESP_GET_BEGIN, SL_MAX_STRING_SIZE )
                    + ESP_MAC_ADDR_SIZE
                    + sl_strnlen( ESP_ACK_BEGIN, SL_MAX_STRING_SIZE )
                    + sl_strnlen( esp_relay_state != ESP_RELAY_OFF ? \
                                        ESP_ACK_RELAY_ON : ESP_ACK_RELAY_OFF
                                , SL_MAX_STRING_SIZE
                                )
                    + sl_strnlen( ESP_GET_FINISH, SL_MAX_STRING_SIZE )
                    , ESP_MAX_BUFF_SIZE
                    );

        esp_hdl.send( esp_hdl.base
                    , esp_buff
                    , sl_strnlen( esp_buff, ESP_MAX_BUFF_SIZE )
                    );

        sl_set_timeout( ESP_TIMEOUT, SL_TIME_MSEC, &timeout );

        do
        {
            is_timeout = sl_is_timeout( timeout );
            ch = bl_uart_getc( esp_hdl.base );
        }
        while (( ch != '>' ) && ( FALSE == is_timeout ));

        if ( FALSE == is_timeout )
        {
            sl_wait( 1, SL_TIME_SEC );
            bl_uart_buff_clear( esp_hdl.base );

            esp_hdl.send( esp_hdl.base
                        , ESP_GET_BEGIN
                        , sl_strnlen( ESP_GET_BEGIN, SL_MAX_STRING_SIZE )
                        );

            esp_hdl.send( esp_hdl.base
                        , esp_mac_addr
                        , ESP_MAC_ADDR_SIZE
                        );

            esp_hdl.send( esp_hdl.base
                        , ESP_ACK_BEGIN
                        , sl_strnlen( ESP_ACK_BEGIN, SL_MAX_STRING_SIZE )
                        );

            if ( ESP_RELAY_OFF != esp_relay_state )
            {
                esp_hdl.send( esp_hdl.base
                            , ESP_ACK_RELAY_ON
                            , sl_strnlen( ESP_ACK_RELAY_ON
                                        , SL_MAX_STRING_SIZE
                                        )
                            );
            }
            else
            {
                esp_hdl.send( esp_hdl.base
                            , ESP_ACK_RELAY_OFF
                            , sl_strnlen( ESP_ACK_RELAY_OFF
                                        , SL_MAX_STRING_SIZE
                                        )
                            );
            }

            esp_hdl.send( esp_hdl.base
                        , ESP_GET_FINISH
                        , sl_strnlen( ESP_GET_FINISH, SL_MAX_STRING_SIZE )
                        );

            /* Wait for 1 second after issuing the AT+CIPSEND command */
            sl_wait( 1, SL_TIME_SEC );

            ret = esp_parse( ESP_SEND_OK, ESP_SEND_OK_SIZE );

            /* Detect server response before checking
             * the connection status
             */

            ret = esp_parse( ESP_HTTP_IPD_START
                           , ESP_HTTP_IPD_START_SIZE
                           );

            esp_hdl.send( esp_hdl.base
                        , ESP_WIFI_STATUS
                        , sl_strnlen( ESP_WIFI_STATUS, SL_MAX_STRING_SIZE )
                        );

            ret = esp_parse( ESP_WIFI_ST_TAG, ESP_WIFI_ST_TAG_SZ );

            if (( ESP_RET_OK == ret )
            && ( esp_buff[ESP_WIFI_ST_TAG_SZ] != ESP_WIFI_ST_DISCONNECTED ))
            {
                esp_hdl.send( esp_hdl.base
                            , (uint8_t*)"AT+CIPCLOSE\r\n"
                            , 13
                            );
            }
        }
        else
        {
            ret = ESP_RET_TIMED_OUT;
        }
    }
    else
    {
        ret = ESP_RET_NO_CONNECTION;
    }

    return ret;
}

void esp_log_error( Esp_Ret ret_code )
{
    Esp_Cfg_t cfg_tmp;

    /* Assume handle is always valid because
     * it is validated during ESP8266 initialization.
     */
    sl_memcpy( &cfg_tmp, esp_hdl.cfg, sizeof(Esp_Cfg_t));

    switch ( ret_code )
    {
        case ESP_RET_NO_CONNECTION:
            cfg_tmp.err.err_connect++;
        break;
        case ESP_RET_PACKET_ERR:
            cfg_tmp.err.err_packet++;
        break;
        case ESP_RET_TIMED_OUT:
            cfg_tmp.err.err_timeout++;
        break;
        default:
            cfg_tmp.err.err_unknown++;
        break;
    }

    bl_flash_write((Esp_Cfg_t*) &cfg_tmp
                  , sizeof (Esp_Cfg_t)
                  , ESP_CFG_DATA_OFFSET
                  );
}

void esp_update_stats( void )
{
    esp_live_stats++;
}

void esp_dump_live_stats( void )
{
    sl_sprintf_d( esp_buff
                , (uint8_t*)"Sent (%d) "
                            "Errors -> packet(%d) connect(%d) "
                            "timeout(%d) unknown(%d)\r"
                , esp_live_stats
                , SL_MAX_STRING_SIZE
                );

    sl_sprintf_d( esp_buff
                , esp_buff
                , esp_hdl.cfg->err.err_packet
                , SL_MAX_STRING_SIZE
                );

    sl_sprintf_d( esp_buff
                , esp_buff
                , esp_hdl.cfg->err.err_connect
                , SL_MAX_STRING_SIZE
                );

    sl_sprintf_d( esp_buff
                , esp_buff
                , esp_hdl.cfg->err.err_timeout
                , SL_MAX_STRING_SIZE
                );

    sl_sprintf_d( esp_buff
                , esp_buff
                , esp_hdl.cfg->err.err_unknown
                , SL_MAX_STRING_SIZE
                );

    bl_uart_send( UART_DBG
                , esp_buff
                , sl_strnlen( esp_buff, SL_MAX_STRING_SIZE )
                );
}

Esp_Err_Log_t esp_get_error_cnt( void )
{
    return esp_hdl.cfg->err;
}

void esp_reset_error_counter( void )
{
    Esp_Cfg_t cfg_tmp;

    sl_memcpy( &cfg_tmp, esp_hdl.cfg, sizeof(Esp_Cfg_t));

    cfg_tmp.err.err_connect = 0;
    cfg_tmp.err.err_packet  = 0;
    cfg_tmp.err.err_timeout = 0;
    cfg_tmp.err.err_unknown = 0;

    bl_flash_write((Esp_Cfg_t*) &cfg_tmp
                  , sizeof (Esp_Cfg_t)
                  , ESP_CFG_DATA_OFFSET
                  );
}

Esp_Relay_State_t esp_relay_get_state( void )
{
    return esp_relay_state;
}

Esp_Ret esp_relay_set_state( Esp_Relay_State_t new_state )
{
    Esp_Ret ret = ESP_RET_OK;

    switch ( new_state )
    {
        case ESP_RELAY_OFF:
        case ESP_RELAY_ON:
            esp_relay_state = new_state;
        break;
        default:
            ret = ESP_RET_INV_MODE;
        break;
    }

    return ret;
}
