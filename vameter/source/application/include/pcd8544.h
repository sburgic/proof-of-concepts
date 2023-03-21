/**
 ** Name
 **   pcd8544.h
 **
 ** Purpose
 **   PCD8544 driver
 **
 ** Revision
 **   26-Oct-2020 (SB) [] Initial
 **/

#ifndef __PCD8544_H__
#define __PCD8544_H__

#include "ptypes.h"

#define PCD8544_SPI      SPI2

/* GPIO mapping */
#define PCD8544_RST_PORT GPIOA
#define PCD8544_RST_PIN  GPIO_PIN_5

#define PCD8544_RST_CLK_ENABLE() \
                do { __HAL_RCC_GPIOA_CLK_ENABLE(); } while(0)

#define PCD8544_DC_PORT GPIOA
#define PCD8544_DC_PIN  GPIO_PIN_4

#define PCD8544_DC_CLK_ENABLE() \
                do { __HAL_RCC_GPIOA_CLK_ENABLE(); } while(0)

#define PCD8544_CS_PORT GPIOB
#define PCD8544_CS_PIN  GPIO_PIN_12

#define PCD8544_CS_CLK_ENABLE() \
                do { __HAL_RCC_GPIOB_CLK_ENABLE(); } while(0)

/* Display presets */
#define PCD8544_LCD_BIAS     0x03 /* Range: 0-7 (0x00-0x07) */
#define PCD8544_LCD_TEMP     0x02 /* Range: 0-3 (0x00-0x03) */
#define PCD8544_LCD_CONTRAST 0x46 /* Range: 0-127 (0x00-0x7F) */

typedef enum
{
    PCD8544_PIXEL_CLEAR = 0,
    PCD8544_PIXEL_SET
} PCD8544_Pixel_t;

typedef enum
{
    PCD8544_FONT_SIZE_5X7 = 0,
    PCD8544_FONT_SIZE_3X5
} PCD8544_Font_Size_t;

typedef enum
{
    PCD8544_INVERT_NO = 0,
    PCD8544_INVERT_YES
} PCD8544_Invert_t;

status_t pcd8544_init( uint8_t contrast );
status_t pcd8544_home( void );
status_t pcd8544_refresh( void );
status_t pcd8544_clear( void );
status_t pcd8544_set_contrast( uint8_t contrast );
void pcd8544_goto_xy( uint8_t x, uint8_t y );
status_t pcd8544_draw_pixel( uint8_t x, uint8_t y, PCD8544_Pixel_t pixel );
status_t pcd8544_invert( PCD8544_Invert_t invert );
void pcd8544_update_area( uint8_t xmin
                        , uint8_t ymin
                        , uint8_t xmax
                        , uint8_t ymax
                        );
status_t pcd8544_putc( char                ch
                     , PCD8544_Pixel_t     color
                     , PCD8544_Font_Size_t size
                     );
status_t pcd8544_puts( char*               ch
                     , PCD8544_Pixel_t     color
                     , PCD8544_Font_Size_t size
                     );
status_t pcd8544_draw_line( uint8_t         x0
                          , uint8_t         y0
                          , uint8_t         x1
                          , uint8_t         y1
                          , PCD8544_Pixel_t color
                          );
status_t pcd8544_draw_rectangle( uint8_t         x0
                               , uint8_t         y0
                               , uint8_t         x1
                               , uint8_t         y1
                               , PCD8544_Pixel_t color
                               );
status_t pcd8544_draw_filled_rectangle( uint8_t         x0
                                      , uint8_t         y0
                                      , uint8_t         x1
                                      , uint8_t         y1
                                      , PCD8544_Pixel_t color
                                      );
status_t pcd8544_draw_circle( int8_t          x0
                            , int8_t          y0
                            , int8_t          r
                            , PCD8544_Pixel_t color
                            );
status_t pcd8544_draw_filled_circle( int8_t          x0
                                   , int8_t          y0
                                   , int8_t          r
                                   , PCD8544_Pixel_t color
                                   );

#endif /* __PCD8544_H__ */
