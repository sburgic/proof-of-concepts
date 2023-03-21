/**
 ** Name
 **   pcd8544.c
 **
 ** Purpose
 **   PCD8544 driver
 **
 ** Revision
 **   26-Oct-2020 (SB) [] Initial
 **/

#include "pcd8544.h"

#include <stm32f1xx_hal.h>

#define PCD8544_SPI_TIMEOUT ((uint16_t)1)

#define PCD8544_WIDTH  84
#define PCD8544_HEIGHT 48

/* General commands */
#define PCD8544_POWERDOWN           0x04
#define PCD8544_ENTRYMODE           0x02
#define PCD8544_EXTENDEDINSTRUCTION 0x01
#define PCD8544_DISPLAYBLANK        0x00
#define PCD8544_DISPLAYNORMAL       0x04
#define PCD8544_DISPLAYALLON        0x01
#define PCD8544_DISPLAYINVERTED     0x05
/* Normal instruction set */
#define PCD8544_FUNCTIONSET         0x20
#define PCD8544_DISPLAYCONTROL      0x08
#define PCD8544_SETYADDR            0x40
#define PCD8544_SETXADDR            0x80
/* Extended instruction set */
#define PCD8544_SETTEMP             0x04
#define PCD8544_SETBIAS             0x10
#define PCD8544_SETVOP              0x80

#define PCD8544_CHAR_5X7_WIDTH      6 /* 5x7 */
#define PCD8544_CHAR_5X7_HEIGHT     8
#define PCD8544_CHAR_3X5_WIDTH      4 /* 3x5 */
#define PCD8544_CHAR_3X5_HEIGHT     6
#define PCD8544_BUFF_SIZE           PCD8544_WIDTH * PCD8544_HEIGHT / 8

typedef enum
{
    PCD8544_WRITE_CMD = 0,
    PCD8544_WRITE_DATA
} PCD8544_Write_t;

static SPI_HandleTypeDef spi_hdl;

static uint8_t pcd8544_buff[PCD8544_BUFF_SIZE];
static uint8_t pcd8544_update_x_min;
static uint8_t pcd8544_update_x_max;
static uint8_t pcd8544_update_y_min;
static uint8_t pcd8544_update_y_max;
static uint8_t pcd8544_x;
static uint8_t pcd8544_y;

static const uint8_t font_5x7 [97][PCD8544_CHAR_5X7_WIDTH] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 }, /* s */
    { 0x00, 0x00, 0x2f, 0x00, 0x00 }, /* ! */
    { 0x00, 0x07, 0x00, 0x07, 0x00 }, /* " */
    { 0x14, 0x7f, 0x14, 0x7f, 0x14 }, /* # */
    { 0x24, 0x2a, 0x7f, 0x2a, 0x12 }, /* $ */
    { 0x32, 0x34, 0x08, 0x16, 0x26 }, /* % */
    { 0x36, 0x49, 0x55, 0x22, 0x50 }, /* & */
    { 0x00, 0x05, 0x03, 0x00, 0x00 }, /* ' */
    { 0x00, 0x1c, 0x22, 0x41, 0x00 }, /* ( */
    { 0x00, 0x41, 0x22, 0x1c, 0x00 }, /* ) */
    { 0x14, 0x08, 0x3E, 0x08, 0x14 }, /* * */
    { 0x08, 0x08, 0x3E, 0x08, 0x08 }, /* + */
    { 0x00, 0x00, 0x50, 0x30, 0x00 }, /* , */
    { 0x10, 0x10, 0x10, 0x10, 0x10 }, /* - */
    { 0x00, 0x60, 0x60, 0x00, 0x00 }, /* . */
    { 0x20, 0x10, 0x08, 0x04, 0x02 }, /* / */
    { 0x3E, 0x51, 0x49, 0x45, 0x3E }, /* 0 */
    { 0x00, 0x42, 0x7F, 0x40, 0x00 }, /* 1 */
    { 0x42, 0x61, 0x51, 0x49, 0x46 }, /* 2 */
    { 0x21, 0x41, 0x45, 0x4B, 0x31 }, /* 3 */
    { 0x18, 0x14, 0x12, 0x7F, 0x10 }, /* 4 */
    { 0x27, 0x45, 0x45, 0x45, 0x39 }, /* 5 */
    { 0x3C, 0x4A, 0x49, 0x49, 0x30 }, /* 6 */
    { 0x01, 0x71, 0x09, 0x05, 0x03 }, /* 7 */
    { 0x36, 0x49, 0x49, 0x49, 0x36 }, /* 8 */
    { 0x06, 0x49, 0x49, 0x29, 0x1E }, /* 9 */
    { 0x00, 0x36, 0x36, 0x00, 0x00 }, /* : */
    { 0x00, 0x56, 0x36, 0x00, 0x00 }, /* ; */
    { 0x08, 0x14, 0x22, 0x41, 0x00 }, /* < */
    { 0x14, 0x14, 0x14, 0x14, 0x14 }, /* = */
    { 0x00, 0x41, 0x22, 0x14, 0x08 }, /* > */
    { 0x02, 0x01, 0x51, 0x09, 0x06 }, /* ? */
    { 0x32, 0x49, 0x59, 0x51, 0x3E }, /* @ */
    { 0x7E, 0x11, 0x11, 0x11, 0x7E }, /* A */
    { 0x7F, 0x49, 0x49, 0x49, 0x36 }, /* B */
    { 0x3E, 0x41, 0x41, 0x41, 0x22 }, /* C */
    { 0x7F, 0x41, 0x41, 0x22, 0x1C }, /* D */
    { 0x7F, 0x49, 0x49, 0x49, 0x41 }, /* E */
    { 0x7F, 0x09, 0x09, 0x09, 0x01 }, /* F */
    { 0x3E, 0x41, 0x49, 0x49, 0x7A }, /* G */
    { 0x7F, 0x08, 0x08, 0x08, 0x7F }, /* H */
    { 0x00, 0x41, 0x7F, 0x41, 0x00 }, /* I */
    { 0x20, 0x40, 0x41, 0x3F, 0x01 }, /* J */
    { 0x7F, 0x08, 0x14, 0x22, 0x41 }, /* K */
    { 0x7F, 0x40, 0x40, 0x40, 0x40 }, /* L */
    { 0x7F, 0x02, 0x0C, 0x02, 0x7F }, /* M */
    { 0x7F, 0x04, 0x08, 0x10, 0x7F }, /* N */
    { 0x3E, 0x41, 0x41, 0x41, 0x3E }, /* O */
    { 0x7F, 0x09, 0x09, 0x09, 0x06 }, /* P */
    { 0x3E, 0x41, 0x51, 0x21, 0x5E }, /* Q */
    { 0x7F, 0x09, 0x19, 0x29, 0x46 }, /* R */
    { 0x46, 0x49, 0x49, 0x49, 0x31 }, /* S */
    { 0x01, 0x01, 0x7F, 0x01, 0x01 }, /* T */
    { 0x3F, 0x40, 0x40, 0x40, 0x3F }, /* U */
    { 0x1F, 0x20, 0x40, 0x20, 0x1F }, /* V */
    { 0x3F, 0x40, 0x38, 0x40, 0x3F }, /* W */
    { 0x63, 0x14, 0x08, 0x14, 0x63 }, /* X */
    { 0x07, 0x08, 0x70, 0x08, 0x07 }, /* Y */
    { 0x61, 0x51, 0x49, 0x45, 0x43 }, /* Z */
    { 0x00, 0x7F, 0x41, 0x41, 0x00 }, /* [ */
    { 0x55, 0x2A, 0x55, 0x2A, 0x55 }, /* 5 */
    { 0x00, 0x41, 0x41, 0x7F, 0x00 }, /* ] */
    { 0x04, 0x02, 0x01, 0x02, 0x04 }, /* ^ */
    { 0x40, 0x40, 0x40, 0x40, 0x40 }, /* _ */
    { 0x00, 0x01, 0x02, 0x04, 0x00 }, /* ' */
    { 0x20, 0x54, 0x54, 0x54, 0x78 }, /* a */
    { 0x7F, 0x48, 0x44, 0x44, 0x38 }, /* b */
    { 0x38, 0x44, 0x44, 0x44, 0x20 }, /* c */
    { 0x38, 0x44, 0x44, 0x48, 0x7F }, /* d */
    { 0x38, 0x54, 0x54, 0x54, 0x18 }, /* e */
    { 0x08, 0x7E, 0x09, 0x01, 0x02 }, /* f */
    { 0x0C, 0x52, 0x52, 0x52, 0x3E }, /* g */
    { 0x7F, 0x08, 0x04, 0x04, 0x78 }, /* h */
    { 0x00, 0x44, 0x7D, 0x40, 0x00 }, /* i */
    { 0x20, 0x40, 0x44, 0x3D, 0x00 }, /* j */
    { 0x7F, 0x10, 0x28, 0x44, 0x00 }, /* k */
    { 0x00, 0x41, 0x7F, 0x40, 0x00 }, /* l */
    { 0x7C, 0x04, 0x18, 0x04, 0x78 }, /* m */
    { 0x7C, 0x08, 0x04, 0x04, 0x78 }, /* n */
    { 0x38, 0x44, 0x44, 0x44, 0x38 }, /* o */
    { 0x7C, 0x14, 0x14, 0x14, 0x08 }, /* p */
    { 0x08, 0x14, 0x14, 0x18, 0x7C }, /* q */
    { 0x7C, 0x08, 0x04, 0x04, 0x08 }, /* r */
    { 0x48, 0x54, 0x54, 0x54, 0x20 }, /* s */
    { 0x04, 0x3F, 0x44, 0x40, 0x20 }, /* t */
    { 0x3C, 0x40, 0x40, 0x20, 0x7C }, /* u */
    { 0x1C, 0x20, 0x40, 0x20, 0x1C }, /* v */
    { 0x3C, 0x40, 0x30, 0x40, 0x3C }, /* w */
    { 0x44, 0x28, 0x10, 0x28, 0x44 }, /* x */
    { 0x0C, 0x50, 0x50, 0x50, 0x3C }, /* y */
    { 0x44, 0x64, 0x54, 0x4C, 0x44 }, /* z */
    { 0x00, 0x7F, 0x3E, 0x1C, 0x08 }, /* > Filled   123 */
    { 0x08, 0x1C, 0x3E, 0x7F, 0x00 }, /* < Filled   124 */
    { 0x08, 0x7C, 0x7E, 0x7C, 0x08 }, /* Arrow up   125 */
    { 0x10, 0x3E, 0x7E, 0x3E, 0x10 }, /* Arrow down 126 */
    { 0x3E, 0x3E, 0x3E, 0x3E, 0x3E }, /* Stop       127 */
    { 0x00, 0x7F, 0x3E, 0x1C, 0x08 }  /* Play       128 */
};

static const uint8_t font_3x5[106][3] =
{
    { 0x00, 0x00, 0x00 }, /* sp - 32 */
    { 0x00, 0x17, 0x00 }, /* ! - 33  */
    { 0x03, 0x00, 0x03 }, /* " - 34  */
    { 0x1F, 0x0A, 0x1F }, /* # - 35  */
    { 0x0A, 0x1F, 0x05 }, /* $ */
    { 0x09, 0x04, 0x12 }, /* % */
    { 0x0F, 0x17, 0x1C }, /* & */
    { 0x00, 0x03, 0x00 }, /* ' */
    { 0x00, 0x0E, 0x11 }, /* ( - 40 */
    { 0x11, 0x0E, 0x00 }, /* ) */
    { 0x05, 0x02, 0x05 }, /* * */
    { 0x04, 0x0E, 0x04 }, /* + */
    { 0x10, 0x08, 0x00 }, /* , */
    { 0x04, 0x04, 0x04 }, /* - - 45 */
    { 0x00, 0x10, 0x00 }, /* . */
    { 0x08, 0x04, 0x02 }, /* / */
    { 0x1F, 0x11, 0x1F }, /* 0 */
    { 0x12, 0x1F, 0x10 }, /* 1 */
    { 0x1D, 0x15, 0x17 }, /* 2 - 50 */
    { 0x11, 0x15, 0x1F }, /* 3 */
    { 0x07, 0x04, 0x1F }, /* 4 */
    { 0x17, 0x15, 0x1D }, /* 5 */
    { 0x1F, 0x15, 0x1D }, /* 6 */
    { 0x01, 0x01, 0x1F }, /* 7 - 55 */
    { 0x1F, 0x15, 0x1F }, /* 8 */
    { 0x17, 0x15, 0x1F }, /* 9 - 57 */
    { 0x00, 0x0A, 0x00 }, /* : */
    { 0x10, 0x0A, 0x00 }, /* ; */
    { 0x04, 0x0A, 0x11 }, /* < - 60 */
    { 0x0A, 0x0A, 0x0A }, /* = */
    { 0x11, 0x0A, 0x04 }, /* > */
    { 0x01, 0x15, 0x03 }, /* ? */
    { 0x0E, 0x15, 0x16 }, /* @ */
    { 0x1E, 0x05, 0x1E }, /* A - 65 */
    { 0x1F, 0x15, 0x0A }, /* B */
    { 0x0E, 0x11, 0x11 }, /* C */
    { 0x1F, 0x11, 0x0E }, /* D */
    { 0x1F, 0x15, 0x15 }, /* E */
    { 0x1F, 0x05, 0x05 }, /* F - 70 */
    { 0x0E, 0x15, 0x1D }, /* G */
    { 0x1F, 0x04, 0x1F }, /* H */
    { 0x11, 0x1F, 0x11 }, /* I */
    { 0x08, 0x10, 0x0F }, /* J */
    { 0x1F, 0x04, 0x1B }, /* K - 75 */
    { 0x1F, 0x10, 0x10 }, /* L */
    { 0x1F, 0x06, 0x1F }, /* M */
    { 0x1F, 0x0E, 0x1F }, /* N */
    { 0x0E, 0x11, 0x0E }, /* O */
    { 0x1F, 0x05, 0x02 }, /* P - 80 */
    { 0x0E, 0x11, 0x1E }, /* Q */
    { 0x1F, 0x0D, 0x16 }, /* R */
    { 0x12, 0x15, 0x09 }, /* S */
    { 0x01, 0x1F, 0x01 }, /* T */
    { 0x0F, 0x10, 0x0F }, /* U - 85 */
    { 0x07, 0x18, 0x07 }, /* V */
    { 0x1F, 0x0C, 0x1F }, /* W */
    { 0x1B, 0x04, 0x1B }, /* X */
    { 0x03, 0x1C, 0x03 }, /* Y */
    { 0x19, 0x15, 0x13 }, /* Z - 90 */
    { 0x1F, 0x11, 0x00 }, /* [ */
    { 0x02, 0x04, 0x08 }, /* 55 - backspace - 92 */
    { 0x00, 0x11, 0x1F }, /* ] */
    { 0x02, 0x01, 0x02 }, /* ^ */
    { 0x10, 0x10, 0x10 }, /* _ - 95 */
    { 0x01, 0x02, 0x00 }, /* ` */
    { 0x1A, 0x16, 0x1C }, /* a */
    { 0x1F, 0x12, 0x0C }, /* b */
    { 0x0C, 0x12, 0x12 }, /* c */
    { 0x0C, 0x12, 0x1F }, /* d - 100 */
    { 0x0C, 0x1A, 0x16 }, /* e */
    { 0x04, 0x1E, 0x05 }, /* f */
    { 0x06, 0x15, 0x0F }, /* g */
    { 0x1F, 0x02, 0x1C }, /* h */
    { 0x00, 0x1D, 0x00 }, /* i - 105 */
    { 0x10, 0x10, 0x0D }, /* j */
    { 0x1F, 0x0C, 0x12 }, /* k */
    { 0x11, 0x1F, 0x10 }, /* l */
    { 0x1E, 0x0E, 0x1E }, /* m */
    { 0x1E, 0x02, 0x1C }, /* n - 110 */
    { 0x0C, 0x12, 0x0C }, /* o */
    { 0x1E, 0x0A, 0x04 }, /* p */
    { 0x04, 0x0A, 0x1E }, /* q */
    { 0x1C, 0x02, 0x02 }, /* r */
    { 0x14, 0x1E, 0x0A }, /* s - 115 */
    { 0x02, 0x1F, 0x12 }, /* t */
    { 0x0E, 0x10, 0x1E }, /* u */
    { 0x0E, 0x10, 0x0E }, /* v */
    { 0x1E, 0x1C, 0x1E }, /* w */
    { 0x12, 0x0C, 0x12 }, /* x - 120 */
    { 0x02, 0x14, 0x1E }, /* y */
    { 0x1A, 0x1E, 0x16 }, /* z */
    { 0x04, 0x1B, 0x11 }, /* { */
    { 0x00, 0x1F, 0x00 }, /* | */
    { 0x11, 0x1B, 0x04 }, /* } */
    { 0x04, 0x06, 0x02 }, /* ~ */
    { 0x1F, 0x1F, 0x1F }, /* delete */
};

static __inline__ void pcd8544_spi_cs_enable( void )
{
    HAL_GPIO_WritePin( PCD8544_CS_PORT, PCD8544_CS_PIN, GPIO_PIN_RESET );
}

static __inline__ void pcd8544_spi_cs_disable( void )
{
    HAL_GPIO_WritePin( PCD8544_CS_PORT, PCD8544_CS_PIN, GPIO_PIN_SET );
}

static void pcd8544_gpio_init( void )
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    PCD8544_RST_CLK_ENABLE();
    PCD8544_DC_CLK_ENABLE();
    PCD8544_CS_CLK_ENABLE();

    GPIO_InitStruct.Pin   = PCD8544_RST_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( PCD8544_RST_PORT, &GPIO_InitStruct );

    GPIO_InitStruct.Pin   = PCD8544_DC_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( PCD8544_DC_PORT, &GPIO_InitStruct );

    GPIO_InitStruct.Pin   = PCD8544_CS_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( PCD8544_CS_PORT, &GPIO_InitStruct );

    pcd8544_spi_cs_disable();
    HAL_GPIO_WritePin( PCD8544_RST_PORT, PCD8544_RST_PIN, GPIO_PIN_SET );
}

static status_t pcd8544_spi_init( SPI_TypeDef* spi )
{
    status_t          ret = STATUS_OK;
    HAL_StatusTypeDef hret;

    spi_hdl.Instance               = spi;
    spi_hdl.Init.Mode              = SPI_MODE_MASTER;
    spi_hdl.Init.Direction         = SPI_DIRECTION_2LINES;
    spi_hdl.Init.DataSize          = SPI_DATASIZE_8BIT;
    spi_hdl.Init.CLKPolarity       = SPI_POLARITY_LOW;
    spi_hdl.Init.CLKPhase          = SPI_PHASE_1EDGE;
    spi_hdl.Init.NSS               = SPI_NSS_SOFT;
    spi_hdl.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    spi_hdl.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    spi_hdl.Init.TIMode            = SPI_TIMODE_DISABLE;
    spi_hdl.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    spi_hdl.Init.CRCPolynomial     = 0x10,

    hret = HAL_SPI_Init( &spi_hdl );

    if ( HAL_OK != hret )
    {
        ret = STATUS_ERROR;
    }

    return ret;
}

static status_t pcd8544_spi_send_byte( uint8_t byte )
{
    status_t          ret = STATUS_OK;
    HAL_StatusTypeDef hret;

    pcd8544_spi_cs_enable();

    hret = HAL_SPI_Transmit( &spi_hdl, &byte, 1, PCD8544_SPI_TIMEOUT );

    if ( HAL_OK != hret)
    {
        ret = STATUS_ERROR;
    }

    pcd8544_spi_cs_disable();

    return ret;
}

static status_t pcd8544_write( PCD8544_Write_t cd, uint8_t data )
{
    status_t ret = STATUS_OK;

    switch ( cd )
    {
        case PCD8544_WRITE_DATA:
            HAL_GPIO_WritePin( PCD8544_DC_PORT, PCD8544_DC_PIN, GPIO_PIN_SET );
            break;
        case PCD8544_WRITE_CMD:
            HAL_GPIO_WritePin( PCD8544_DC_PORT
                             , PCD8544_DC_PIN
                             , GPIO_PIN_RESET
                             );
            break;
        default:
            ret = STATUS_ERROR;
            break;
    }

    ret = pcd8544_spi_send_byte( data );

    return ret;
}

status_t pcd8544_init( uint8_t contrast )
{
    status_t ret;

    pcd8544_gpio_init();
    ret = pcd8544_spi_init( PCD8544_SPI );

    if ( STATUS_ERROR != ret )
    {
        /* Reset the device */
        HAL_GPIO_WritePin( PCD8544_RST_PORT, PCD8544_RST_PIN, GPIO_PIN_RESET );
        HAL_Delay( 10 );
        HAL_GPIO_WritePin( PCD8544_RST_PORT, PCD8544_RST_PIN, GPIO_PIN_SET );

        /* Go in extended mode */
        ret = pcd8544_write( PCD8544_WRITE_CMD
                           , PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION
                           );

        /* LCD bias select */
        ret |= pcd8544_write( PCD8544_WRITE_CMD, PCD8544_SETBIAS | 0x4 );

        ret |= pcd8544_set_contrast( contrast );

        /* Set display to normal */
        ret |= pcd8544_write( PCD8544_WRITE_CMD
                            , PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL
                            );

        /* Set cursor to home position */
        ret |= pcd8544_home();

        /* Normal display */
        ret |= pcd8544_write( PCD8544_WRITE_CMD
                            , PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL
                            );

        /* Clear display */
        ret |= pcd8544_clear();
    }

    return ret;
}

status_t pcd8544_home( void )
{
    status_t ret;

    ret  = pcd8544_write( PCD8544_WRITE_CMD, PCD8544_SETXADDR | 0 );
    ret |= pcd8544_write( PCD8544_WRITE_CMD, PCD8544_SETYADDR | 0 );

    return ret;
}

status_t pcd8544_clear( void )
{
    status_t ret;
    uint16_t i;

    ret = pcd8544_home();

    for ( i = 0; i < PCD8544_BUFF_SIZE; i++ )
    {
        pcd8544_buff[i] = 0x00;
    }

    pcd8544_goto_xy( 0, 0 );
    pcd8544_update_area( 0, 0, PCD8544_WIDTH - 1, PCD8544_HEIGHT - 1 );
    ret |= pcd8544_refresh();

    return ret;
}

status_t pcd8544_set_contrast( uint8_t contrast )
{
    status_t ret;

    ret = pcd8544_write( PCD8544_WRITE_CMD
                       , PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION
                       );

    if ( contrast > 0x7F )
    {
        contrast = 0x7F;
    }

    ret |= pcd8544_write( PCD8544_WRITE_CMD, PCD8544_SETVOP | contrast );
    ret |= pcd8544_write( PCD8544_WRITE_CMD, PCD8544_FUNCTIONSET );

    return ret;
}

status_t pcd8544_draw_pixel( uint8_t x, uint8_t y, PCD8544_Pixel_t pixel )
{
    status_t ret = STATUS_OK;

    if (( x >= PCD8544_WIDTH ) || ( y >= PCD8544_HEIGHT ))
    {
        ret = STATUS_ERROR;
    }
    else
    {
        if ( PCD8544_PIXEL_CLEAR != pixel )
        {
            pcd8544_buff[x + ( y / 8 ) * PCD8544_WIDTH] |= 1 << ( y % 8 );
        }
        else
        {
            pcd8544_buff[x + (y / 8) * PCD8544_WIDTH] &= ~( 1 << ( y % 8 ));
        }

        pcd8544_update_area( x, y, x, y );
    }

    return ret;
}

status_t pcd8544_invert( PCD8544_Invert_t invert )
{
    status_t ret;

    if ( PCD8544_INVERT_NO != invert )
    {
        ret = pcd8544_write( PCD8544_WRITE_CMD
                           , PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYINVERTED
                           );
    }
    else
    {
        ret = pcd8544_write( PCD8544_WRITE_CMD
                           , PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL
                           );
    }

    return ret;
}

status_t pcd8544_refresh( void )
{
    status_t ret = STATUS_OK;
    uint8_t  i;
    uint8_t  j;

    for ( i = 0; i < 6; i++ )
    {
        /* Not in range yet */
        if ( pcd8544_update_y_min > (( i + 1 ) * 8 ))
        {
            continue;
        }
        /* Over range, stop */
        if (( i * 8 ) > pcd8544_update_y_max )
        {
            break;
        }

        ret |= pcd8544_write( PCD8544_WRITE_CMD, PCD8544_SETYADDR | i );
        ret |= pcd8544_write( PCD8544_WRITE_CMD
                            , PCD8544_SETXADDR | pcd8544_update_x_min
                            );

        for ( j = pcd8544_update_x_min; j <= pcd8544_update_x_max; j++ )
        {
            ret |= pcd8544_write( PCD8544_WRITE_DATA
                                , pcd8544_buff[( i * PCD8544_WIDTH ) + j]
                                );
        }
    }

    pcd8544_update_x_min = PCD8544_WIDTH - 1;
    pcd8544_update_x_max = 0;
    pcd8544_update_y_min = PCD8544_HEIGHT - 1;
    pcd8544_update_y_max = 0;

    return ret;
}

void pcd8544_update_area( uint8_t xmin
                        , uint8_t ymin
                        , uint8_t xmax
                        , uint8_t ymax
                        )
{
    if ( xmin < pcd8544_update_x_min )
    {
        pcd8544_update_x_min = xmin;
    }
    if ( xmax > pcd8544_update_x_max )
    {
        pcd8544_update_x_max = xmax;
    }
    if ( ymin < pcd8544_update_y_min )
    {
        pcd8544_update_y_min = ymin;
    }
    if ( ymax > pcd8544_update_y_max )
    {
        pcd8544_update_y_max = ymax;
    }
}

void pcd8544_goto_xy( uint8_t x, uint8_t y )
{
    pcd8544_x = x;
    pcd8544_y = y;
}

status_t pcd8544_putc( char                ch
                     , PCD8544_Pixel_t     color
                     , PCD8544_Font_Size_t size
                     )
{
    status_t ret;
    uint8_t c_height;
    uint8_t c_width;
    uint8_t i;
    uint8_t b;
    uint8_t j;

    if ( PCD8544_FONT_SIZE_3X5 == size )
    {
        c_width  = PCD8544_CHAR_3X5_WIDTH;
        c_height = PCD8544_CHAR_3X5_HEIGHT;
    }
    else
    {
        c_width  = PCD8544_CHAR_5X7_WIDTH;
        c_height = PCD8544_CHAR_5X7_HEIGHT;
    }

    if (( pcd8544_x + c_width ) > PCD8544_WIDTH )
    {
        /* If at the end of a line of display, go to new line
         * and set x to 0 position
         */
        pcd8544_y += c_height;
        pcd8544_x  = 0;
    }

    for ( i = 0; i < c_width - 1; i++ )
    {
        if ( ch < 32 )
        {   b = 0; /* Avoid compiler warning */
            /* Example: b = _custom_chars[_font_size][(uint8_t)chr][i]; */
        }
        else if ( PCD8544_FONT_SIZE_3X5 == size )
        {
            b = font_3x5[ch - 32][i];
        }
        else
        {
            b = font_5x7[ch - 32][i];
        }

        if ( b == 0x00 && (( ch != 0 ) && ( ch != 32 )))
        {
            continue;
        }

        for ( j = 0; j < c_height; j++ )
        {
            if ( PCD8544_PIXEL_SET == color )
            {
                ret = pcd8544_draw_pixel
                            ( pcd8544_x
                            , ( pcd8544_y + j )
                            , (( b >> j ) & 1 ) ? \
                              PCD8544_PIXEL_SET : PCD8544_PIXEL_CLEAR
                            );
            }
            else
            {
                ret = pcd8544_draw_pixel
                            ( pcd8544_x
                            , ( pcd8544_y + j )
                            , (( b >> j ) & 1 ) ? \
                              PCD8544_PIXEL_CLEAR : PCD8544_PIXEL_SET
                            );
            }
        }
        pcd8544_x++;
    }
    pcd8544_x++;

    return ret;
}

status_t pcd8544_puts( char*               ch
                     , PCD8544_Pixel_t     color
                     , PCD8544_Font_Size_t size
                     )
{
    status_t ret = STATUS_ERROR;

    if ( NULL != ch )
    {
        while ( *ch )
        {
            ret = pcd8544_putc( *ch++, color, size );
        }
    }

    return ret;
}

status_t pcd8544_draw_line( uint8_t         x0
                          , uint8_t         y0
                          , uint8_t         x1
                          , uint8_t         y1
                          , PCD8544_Pixel_t color
                          )
{
    status_t ret = STATUS_OK;
    int16_t  dx;
    int16_t  dy;
    int16_t  temp;

    if ( x0 > x1 )
    {
        temp = x1;
        x1   = x0;
        x0   = temp;
    }

    if ( y0 > y1 )
    {
        temp = y1;
        y1   = y0;
        y0   = temp;
    }

    dx = x1 - x0;
    dy = y1 - y0;

    if (( 0 == dx) || ( 0 == dy ))
    {
        if ( 0 == dx )
        {
            do
            {
                ret |= pcd8544_draw_pixel( x0, y0, color );
                y0++;
            } while ( y1 >= y0 );
        }
        else
        {
            do
            {
                ret |= pcd8544_draw_pixel( x0, y0, color );
                x0++;
            } while ( x1 >= x0 );
        }
    }
    else
    {
        /* Based on Bresenham's line algorithm */
        if ( dx > dy )
        {
            temp = 2 * dy - dx;
            while ( x0 != x1 )
            {
                ret |= pcd8544_draw_pixel( x0, y0, color );
                x0++;

                if ( temp > 0 )
                {
                    y0++;
                    temp += ( 2 * dy - 2 * dx );
                }
                else
                {
                    temp += ( 2 * dy );
                }
            }

            ret |= pcd8544_draw_pixel( x0, y0, color );
        }
        else
        {
            temp = 2 * dx - dy;
            while ( y0 != y1 )
            {
                ret |= pcd8544_draw_pixel( x0, y0, color );
                y0++;

                if ( temp > 0 )
                {
                    x0++;
                    temp += ( 2 * dy - 2 * dx );
                }
                else
                {
                    temp += ( 2 * dy );
                }
            }

            ret |= pcd8544_draw_pixel( x0, y0, color );
        }
    }

    return ret;
}

status_t pcd8544_draw_rectangle( uint8_t         x0
                               , uint8_t         y0
                               , uint8_t         x1
                               , uint8_t         y1
                               , PCD8544_Pixel_t color
                               )
{
    status_t ret;

    ret  = pcd8544_draw_line( x0, y0, x1, y0, color ); /* Top */
    ret |= pcd8544_draw_line( x0, y0, x0, y1, color ); /* Left */
    ret |= pcd8544_draw_line( x1, y0, x1, y1, color ); /* Right */
    ret |= pcd8544_draw_line( x0, y1, x1, y1, color ); /* Bottom */

    return ret;
}

status_t pcd8544_draw_filled_rectangle( uint8_t         x0
                                      , uint8_t         y0
                                      , uint8_t         x1
                                      , uint8_t         y1
                                      , PCD8544_Pixel_t color
                                      )
{
    status_t ret = STATUS_OK;

    for ( ; y0 < y1; y0++ )
    {
        ret |= pcd8544_draw_line( x0, y0, x1, y0, color );
    }

    return ret;
}

status_t pcd8544_draw_circle( int8_t          x0
                            , int8_t          y0
                            , int8_t          r
                            , PCD8544_Pixel_t color
                            )
{
    status_t ret;
    int16_t  f     = 1 - r;
    int16_t  ddf_x = 1;
    int16_t  ddf_y = -2 * r;
    int16_t  x = 0;
    int16_t  y = r;

    ret  = pcd8544_draw_pixel( x0, y0 + r, color );
    ret |= pcd8544_draw_pixel( x0, y0 - r, color );
    ret |= pcd8544_draw_pixel( x0 + r, y0, color );
    ret |= pcd8544_draw_pixel( x0 - r, y0, color );

    while ( x < y )
    {
        if ( f >= 0 )
        {
            y--;
            ddf_y += 2;
            f     += ddf_y;
        }

        x++;
        ddf_x += 2;
        f     += ddf_x;

        ret |= pcd8544_draw_pixel( x0 + x, y0 + y, color );
        ret |= pcd8544_draw_pixel( x0 - x, y0 + y, color );
        ret |= pcd8544_draw_pixel( x0 + x, y0 - y, color );
        ret |= pcd8544_draw_pixel( x0 - x, y0 - y, color );

        ret |= pcd8544_draw_pixel( x0 + y, y0 + x, color );
        ret |= pcd8544_draw_pixel( x0 - y, y0 + x, color );
        ret |= pcd8544_draw_pixel( x0 + y, y0 - x, color );
        ret |= pcd8544_draw_pixel( x0 - y, y0 - x, color );
    }

    return ret;
}

status_t pcd8544_draw_filled_circle( int8_t          x0
                                   , int8_t          y0
                                   , int8_t          r
                                   , PCD8544_Pixel_t color
                                   )
{
    status_t ret;
    int16_t  f     = 1 - r;
    int16_t  ddf_x = 1;
    int16_t  ddf_y = -2 * r;
    int16_t  x     = 0;
    int16_t  y     = r;

    ret  = pcd8544_draw_pixel( x0, y0 + r, color );
    ret |= pcd8544_draw_pixel( x0, y0 - r, color );
    ret |= pcd8544_draw_pixel( x0 + r, y0, color );
    ret |= pcd8544_draw_pixel( x0 - r, y0, color );
    ret |= pcd8544_draw_line( x0 - r, y0, x0 + r, y0, color );

    while ( x < y )
    {
        if ( f >= 0 )
        {
            y--;
            ddf_y += 2;
            f     += ddf_y;
        }

        x++;
        ddf_x += 2;
        f     += ddf_x;

        ret |= pcd8544_draw_line( x0 - x, y0 + y, x0 + x, y0 + y, color );
        ret |= pcd8544_draw_line( x0 + x, y0 - y, x0 - x, y0 - y, color );

        ret |= pcd8544_draw_line( x0 + y, y0 + x, x0 - y, y0 + x, color );
        ret |= pcd8544_draw_line( x0 + y, y0 - x, x0 - y, y0 - x, color );
    }

    return ret;
}

/* This function is implemented here on purpose!!
 * It is inkoved by HAL_SPI_Init()
 */
void HAL_SPI_MspInit( SPI_HandleTypeDef* spi_hdl )
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if ( SPI2 == spi_hdl->Instance )
    {
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* SPI2 GPIO Configuration
         * PB15     ------> SPI2_MOSI
         * PB14     ------> SPI2_MISO
         * PB13     ------> SPI2_SCK
         */
        GPIO_InitStruct.Pin       = GPIO_PIN_15 | GPIO_PIN_13;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

        GPIO_InitStruct.Pin       = GPIO_PIN_14;
        GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
    }
}
