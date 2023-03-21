/**
  ******************************************************************************
  * @file    application/include/esp8266.h
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    25-Mar-2019
  * @brief   ESP8266 Wifi module defines
  ******************************************************************************
 */

#ifndef ESP8266_H
#define ESP8266_H

#include "types.h"
#include "bl_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_UART_SPEED (115200)

#define ESP_HTTP_TYPE_POST 1

#ifndef ESP_HTTP_TYPE_POST
  #define ESP_HTTP_TYPE_POST 0
#endif

#define ESP_MAX_BUFF_SIZE   (256) /* ESP8266 buffer size */
#define ESP_MAX_CONNECTIONS (4)    /* No of max connections in AP mode */
#define ESP_NO_OF_RETRIES   ((uint8_t)5)

/*
 * ESP8266 Basic
 */
#define ESP_AT       (uint8_t*)"AT\r\n"     /* Returns OK if alive */
#define ESP_RESET    (uint8_t*)"AT+RST\r\n" /* Reset device */
#define ESP_VERSION  (uint8_t*)"AT+GMR\r\n" /* Firwmare ver. number */
#define ESP_ECHO_ON  (uint8_t*)"ATE1\r\n"   /* Echo on */
#define ESP_ECHO_OFF (uint8_t*)"ATE0\r\n"   /* Echo off */

/*
 * ESP8266 success messages
 */
#define ESP_AT_OK            (uint8_t*)"OK\r\n"
#define ESP_AT_OK_SIZE       (4)
#define ESP_SEND_OK          (uint8_t*)"SEND OK\r\n"
#define ESP_SEND_OK_SIZE     (9)
#define ESP_CLOSED           (uint8_t*)"CLOSED\r\n"
#define ESP_CLOSED_SIZE      (8)
#define ESP_WIFI_CON         (uint8_t*)"WIFI CONNECTED\r\n"
#define ESP_WIFI_CON_SIZE    (16)
#define ESP_WIFI_GOT_IP      (uint8_t*)"WIFI GOT IP\r\n"
#define ESP_WIFI_GOT_IP_SIZE (13)

/*
 * ESP8266 Wifi layer
 */
#define ESP_WIFI_MODE_CL    (uint8_t*)"AT+CWMODE=1\r\n"  /* Client mode */
#define ESP_WIFI_MODE_AP    (uint8_t*)"AT+CWMODE=2\r\n"  /* AP mode */
#define ESP_WIFI_MODE_CL_AP (uint8_t*)"AT+CWMODE=3\r\n"  /* Dual mode */
#define ESP_WIFI_MUX_ON     (uint8_t*)"AT+CIPMUX=1\r\n"  /* Allow multiple
                                                          * connections
                                                          */
#define ESP_WIFI_MUX_OFF     (uint8_t*)"AT+CIPMUX=0\r\n" /* Disable multiple
                                                          * connections
                                                          */
#define ESP_WIFI_TCP_SERVER (uint8_t*)"AT+CIPSERVER=1,80\r\n" /* Create TCP
                                                               * server port 80 
                                                               */
#define ESP_HTTP_CIP_SEND   (uint8_t*)"AT+CIPSEND=%d,%d\r\n"  /* Send data */
#define ESP_HTTP_CIP_SEND_L (uint8_t*)"AT+CIPSEND=%d\r\n"     /* Send data with
                                                               * only length
                                                               * provided
                                                               */
#define ESP_WIFI_CIPCLOSE   (uint8_t*)"AT+CIPCLOSE=%d\r\n"    /* Close 
                                                               * connection
                                                               */
#define ESP_WIFI_CONNECT    (uint8_t*)"AT+CWJAP_CUR=\""  /* Connect to AP */
#define ESP_WIFI_STATUS     (uint8_t*)"AT+CIPSTATUS\r\n" /* Check connection
                                                          * status
                                                          */
#define ESP_WIFI_ST_TAG             (uint8_t*)"STATUS:"
#define ESP_WIFI_ST_TAG_SZ          (7)
#define ESP_WIFI_ST_DISCONNECTED    (uint8_t)'4'
#define ESP_LIST_AVAILABLE_AP       (uint8_t*)"AT+CWLAP\r\n"

/* Wifi operations - This should be implemented in a dedicated file
 * but it would require ESP main code refactoring.
 */
#define ESP_OPEN_TCP_TO_SERVER (uint8_t*) \
                            "AT+CIPSTART=\"TCP\",\"62.68.97.44\",8080\r\n"

#define ESP_GET_BEGIN          (uint8_t*)"GET /temp/templog.php?serial="
#define ESP_TEMP_BEGIN         (uint8_t*)"&temp="
#define ESP_MEMO_BEGIN         (uint8_t*)"&memo="
#define ESP_GET_FINISH         (uint8_t*)" HTTP/1.0\r\n\r\n"
#define ESP_HTTP_OK            (uint8_t*)"HTTP/1.1 200 OK"
#define ESP_MEMO_DATA_SIZE     (23) /* Assume max no of digits */
#define ESP_ACK_BEGIN          (uint8_t*)"&ack="
#define ESP_ACK_RELAY_ON       (uint8_t*)"relay_on"
#define ESP_ACK_RELAY_OFF      (uint8_t*)"relay_off"
#define ESP_POST_BEGIN         (uint8_t*) \
"POST /temp/templogpost.php HTTP/1.0\r\n\
Host: 62.68.97.44:8080\r\n\
Content-Length: 64\r\n\
Upgrade-Insecure-Requests: 1\r\n\
Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n\r\n\
serial="
#define ESP_POST_FINISH         (uint8_t*)"\r\n"

/*
 * ESP8266 MAC address defines
 */
#define ESP_WIFI_GET_MAC_AP      (uint8_t*)"AT+CIPAPMAC_DEF?\r\n"
#define ESP_WIFI_GET_MAC_ST      (uint8_t*)"AT+CIPSTAMAC_DEF?\r\n"
#define ESP_WIFI_MAC_TAG_AP      (uint8_t*)"+CIPAPMAC_DEF:\""
#define ESP_WIFI_MAC_TAG_ST      (uint8_t*)"+CIPSTAMAC_DEF:\""
#define ESP_WIFI_SET_MAC_AP      (uint8_t*)"AT+CIPAPMAC_DEF=\""
#define ESP_WIFI_SET_MAC_ST      (uint8_t*)"AT+CIPSTAMAC_DEF=\""
#define ESP_WIFI_MAC_TAG_AP_SIZE (15)
#define ESP_WIFI_MAC_TAG_ST_SIZE (16)
#define ESP_MAC_ADDR_REPLY_SIZE  (35)
#define ESP_MAC_ADDR_SIZE        (17)

/*
 * IP address defines
 */
#define ESP_WIFI_SET_IP (uint8_t*)\
            "AT+CIPAP_DEF=\"192.168.1.1\",\"192.168.1.1\",\"255.255.255.0\"\r\n"

/*
 * DHCP defines
 */
#define ESP_WIFI_DHCP_ON (uint8_t*)"AT+CWDHCP_DEF=0,1\r\n" /* Turn on DHCP */

/*
 * ESP lib configuration
 */
#define ESP_CFG_DATA_OFFSET (0x100)
#define ESP_CFG_DATA_ADDR   (volatile uint32_t)\
                            ( BL_FLASH_USER_PAGE_ADDR + ESP_CFG_DATA_OFFSET )

#define ESP_CFG_HTML_IDX_ADDR (volatile uint32_t)( ADDR_FLASH_PAGE_61)
#define ESP_CFG_HTML_IDX_SIZE ((uint32_t)905)
#define ESP_CFG_HTML_LOGIN_SENT_ADDR (volatile uint32_t)( ADDR_FLASH_PAGE_60)
#define ESP_CFG_HTML_LOGIN_SENT_SIZE ((uint32_t)292)

/*
 * HTTP defines
 */
#define ESP_HTTP_IPD_MAX_SIZE   (16)
#define ESP_HTTP_IPD_START      (uint8_t*)"+IPD,"
#define ESP_HTTP_IPD_START_SIZE (5)
#define ESP_HTTP_ID_SIZE        (1) /* Assume ID is one digit */
#define ESP_HTTP_LEN_START      (7)
#define ESP_HTTP_LEN_SIZE       (4) /* Assume maximum 4 digits */
#define ESP_HTTP_HOST_IP        (uint8_t*)"Host: 192.168.1.1\r\n"
#define ESP_HTTP_HOST_IP_SIZE   (19)

#define ESP_HTTP_NETWORK_TAG       (uint8_t*)"name=\"network\"\r\n\r\n"
#define ESP_HTTP_PASSWORD_TAG      (uint8_t*)"name=\"password\"\r\n\r\n"
#define ESP_HTTP_RELAY_TAG         (uint8_t*)"!!!relay_"
#define ESP_HTTP_NETWORK_TAG_SIZE  (18)
#define ESP_HTTP_PASSWORD_TAG_SIZE (19)
#define ESP_HTTP_RELAY_TAG_SIZE    (9)
#define ESP_HTTP_RELAY_ST_MAX_SIZE (3)

#define ESP_WIFI_SSID_SIZE         (32)
#define ESP_WIFI_PASS_SIZE         (32)
#define ESP_WIFI_SERVER_SIZE       (32)

#define ESP_TEMP_SEND_REPEAT       (30) /* Time in seconds */

typedef HAL_Ret ( *Esp_Send )( UART_Base, uint8_t*, uint16_t );
typedef HAL_Ret ( *Esp_Rec )( UART_Base, uint8_t*, uint16_t, uint16_t* );

/* ESP operating mode */
typedef enum
{
    ESP_MODE_NONE = 0,
    ESP_MODE_CLIENT,
    ESP_MODE_AP
} Esp_Mode;

typedef struct
{
    uint16_t err_connect;
    uint16_t err_packet;
    uint16_t err_timeout;
    uint16_t err_unknown;
} Esp_Err_Log_t;

typedef struct
{
    uint8_t       ssid[ESP_WIFI_SSID_SIZE];
    uint8_t       password[ESP_WIFI_PASS_SIZE];
    uint8_t       server[ESP_WIFI_SERVER_SIZE];
    uint8_t*      html_idx_page;
    uint32_t      html_idx_size;
    uint8_t*      html_login_sent_page;
    uint32_t      html_login_sent_size;
    Esp_Mode      mode;
    Esp_Err_Log_t err;
} Esp_Cfg_t;

typedef struct
{
    Bl_Uart_t* uart_data;
    UART_Base  base;
    Esp_Cfg_t* cfg;
    Esp_Send   send;
    Esp_Rec    receive;
} Esp_t;

typedef enum
{
    ESP_RET_OK = 0,
    ESP_RET_INV_MODE,
    ESP_RET_INV_HDL,
    ESP_RET_NOT_AVAILABLE,
    ESP_RET_TIMED_OUT,
    ESP_RET_NO_CONNECTION,
    ESP_RET_SSID_FAILED,
    ESP_RET_PACKET_ERR
} Esp_Ret;

typedef enum
{
    ESP_RELAY_OFF = 0,
    ESP_RELAY_ON
} Esp_Relay_State_t;

typedef Esp_t Esp_Hdl;

/* Initialize and attach ESP to UART base. 
 * Selected UART needs to be initialized first.
 */
Esp_Ret esp_init( UART_Base base );

/* Set ESP module operating mode */
Esp_Ret esp_set_mode( Esp_Mode mode );

/* Get ESP module operating mode */
Esp_Mode esp_get_mode( void );
    
/* Dump ESP module firmware info */
Esp_Ret esp_dump_fw_info( void );

/* Dump available APs */
Esp_Ret esp_dump_available_ap( void );

/* Set SSID for AP mode */
Esp_Ret esp_set_ssid( void );

/* Enable DHCP */
Esp_Ret esp_enable_dhcp( void );

/* Create TCP server */
Esp_Ret esp_create_tcp_server( void );

/* Listen for HTTP requests */
Esp_Ret esp_tcp_listen( void );

/* Load device configuration */
Esp_Ret esp_load_cfg( void );

/* Detect incoming connection */
Esp_Ret esp_detect_incoming( uint8_t* connection_id );

/* Get ESP8266 MAC address */
Esp_Ret esp_get_mac( uint8_t* buff, Esp_Mode mode );

/* Set ESP8266 MAC address */
Esp_Ret esp_set_mac( uint8_t* buff, Esp_Mode mode );

/* Close all active connections */
Esp_Ret esp_close_all_connections( void );

/* Connect to Wifi */
Esp_Ret esp_connect_to_ap( uint8_t* ssid, uint8_t* password );

/* Send temperature to the server - This should be implemented in a dedicated
 * file but it would require ESP main code refactoring.
 */
Esp_Ret esp_send_temp( uint8_t* temp );

/* Send ACK to the server */
Esp_Ret esp_send_ack( void );

/* Log error */
void esp_log_error( Esp_Ret ret_code );

/* Log successful send/receive of the package */
void esp_update_stats( void );

/* Dump live statistics */
void esp_dump_live_stats( void );

/* Get error counter */
Esp_Err_Log_t esp_get_error_cnt( void );

/* Reset error counter */
void esp_reset_error_counter( void );

/* Return relay state */
Esp_Relay_State_t esp_relay_get_state( void );

/* Set new relay state */
Esp_Ret esp_relay_set_state( Esp_Relay_State_t new_state );

#ifdef __cplusplus
}
#endif

#endif /* ESP8266_H */
