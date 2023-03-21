/**
  ******************************************************************************
  * @file    hal/one_wire/src/one_wire.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.1
  * @date    25-Mar-2019
  * @brief   1-Wire driver
  ******************************************************************************
 */

#include "one_wire.h"
#include "stm32f100xb.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_gpio.h"
#include "types.h"
#include "sl_handle.h"
#include "sl_string.h"

/* Wait for pin to become stable retries count */
#define OW_RESET_RETRIES_CNT                ((uint8_t)125)
/* GPIO input floating mode */
#define OW_INPUT_FLOATING_MODE              ((uint32_t)0x04)
/* GPIO output drain mode, high speed */
#define OW_OUTPUT_OPEN_DRAIN_MODE_50MHZ     ((uint32_t)0x07)
/* Number of cycles per loop - needs update for other frequencies */
#define OW_CYCLES_PER_LOOP  (3)

/* Check "Maxim Integrated 1-Wire Application Note 126" for details.
 * Delay values need to be updated for other CPU frequencies.
 */ 
#define OW_DELAY_A          (138)       /**< 6us   (+1.0)   */
#define OW_DELAY_B          (1520)      /**< 64us  (+7.5)   */
#define OW_DELAY_C          (1420)      /**< 60us  (+7.5)   */
#define OW_DELAY_D          (236)       /**< 10us  (+2.5)   */
#define OW_DELAY_E          (216)       /**< 9us   (+1.0)   */
#define OW_DELAY_F          (1320)      /**< 55us  (+7.0)   */
#define OW_DELAY_H          (11600)     /**< 480us (+70.0)  */
#define OW_DELAY_I          (1700)      /**< 70us  (+8.5)   */
#define OW_DELAY_J          (9840)      /**< 410us (+40.0)  */

/* Wait cycles used for delay in us */
static inline void ow_wait_cycles ( uint32_t n )
{
    uint32_t l = n/OW_CYCLES_PER_LOOP;
    asm volatile( "0:" "SUBS %[count], 1;" "BNE 0b;" :[count]"+r"(l) );
}

/* Initialize clock for GPIO port containing 1-Wire bus */
static HAL_Ret ow_init_gpio_clk( GPIO_TypeDef* port );
/* Set 1-Wire GPIO pin as input */
static void     ow_set_input ( One_Wire* ow );
/* Set 1-Wire GPIO pin as output */
static void     ow_set_output ( One_Wire* ow );
/* Read 1-Wire GPIO pin state */
static uint8_t  ow_read_pin ( One_Wire* ow );
/* Write 1-Wire GPIO pin high */
static void     ow_write_high ( One_Wire* ow );
/* Write 1-Wire GPIO pin low */
static void     ow_write_low ( One_Wire* ow );
/* Global interrupt enable */
static void     ow_enable_it ( void );
/* Global interrupt disable */
static void     ow_disable_it ( void );
/* Reset 1-Wire search state */
static void     ow_reset_search_state ( One_Wire* ow );
/* Write bit to 1-Wire bus */
static void     ow_write_bit ( One_Wire* ow, uint8_t bit );
/* Read bit from 1-Wire bus */
static uint8_t  ow_read_bit ( One_Wire* ow );


/* 1-Wire CRC8 lookup table */
static const uint8_t crc_lookup_tbl[] = 
{
      0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
    157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
     35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
    190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
     70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
    219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
    101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
    248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
    140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
     17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
    175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
     50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
    202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
     87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
    233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
    116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};


static uint8_t found_rom[OW_MAX_DEVICES][OW_ROM_SIZE];


HAL_Ret ow_init ( One_Wire* ow, GPIO_TypeDef* gpio_port, uint16_t gpio_pin )
{
    HAL_Ret         ret = HAL_INV_HDL;
    HAL_GPIO_Data   gpio = {0};
    uint32_t        pin_position = 0;
    uint32_t        pos = 0;
    uint32_t        current_pin = 0;
    uint8_t         reg_shift = 0;
    
    if ( HDL_IS_VALID( ow ) )
    {
        ret = ow_init_gpio_clk ( gpio_port );

        if ( HAL_OK == ret )
        {
            /* First initialization. Pin configuration is changed using atomic
             * operations by ow_set_input() and ow_set_output() further in 
             * program execution
             */
            gpio.Pin    = gpio_pin;
            gpio.Speed  = GPIO_SPEED_FREQ_HIGH;
            gpio.Mode   = GPIO_MODE_INPUT;

            HAL_GPIO_Init ( gpio_port, &gpio );

            ow->bitmask  = gpio_pin;
            ow->port     = gpio_port;

            /* Pin number between 0-7 */
            if ( ( gpio_pin & (uint32_t)0xFF ) > ZERO )
            {
                ow->reg = &gpio_port->CRL;

                for ( pin_position = 0; pin_position < 8; pin_position++ )
                {
                    pos = ( (uint32_t)0x01 ) << pin_position;
                    current_pin = (uint16_t)( ( gpio_pin ) & pos);

                    if ( pos == current_pin )
                    {
                        reg_shift = pin_position << 2;
                        ow->reg_mask = ((uint32_t)0x0F) << reg_shift;
                        break;
                    }
                }
            }
            /* Pin number between 8-15 */
            else
            {
                ow->reg = &gpio_port->CRH;

                for ( pin_position = 0; pin_position < 8; pin_position++ )
                {
                    pos = ( (uint32_t)0x01 << ( pin_position + 8 ) );
                    reg_shift = pin_position << 2;
                    ow->reg_mask = ( (uint32_t)0x0F << reg_shift );
                    break;
                }
            }

             /* Set CNF and MODE GPIO registers  */

            ow->input_mask  = ((( OW_INPUT_FLOATING_MODE ) << reg_shift )
                              & ow->reg_mask
                              );

            ow->output_mask =
                    ((( OW_OUTPUT_OPEN_DRAIN_MODE_50MHZ ) << reg_shift )
                    & ow->reg_mask
                    );

            ow_reset_search_state ( ow );
        }
    }

    return ret;
}


bool_t ow_reset ( One_Wire* ow )
{
    bool_t  ret = TRUE;
    uint8_t retries = OW_RESET_RETRIES_CNT;
    uint8_t read;
    
    ow_disable_it ();
    ow_set_input ( ow );
    ow_enable_it ();
    
    /* Wait for pin to become stable */
    do
    {
        --retries;
        ow_wait_cycles ( OW_DELAY_A );
        read = ow_read_pin ( ow );
        
        if ( ZERO == retries )
        {
            ret = FALSE;
            break;
        }
    }
    while ( ZERO == read );
           
    if ( FALSE != ret )
    {
        ow_disable_it ();
        ow_set_output ( ow );
        ow_write_low ( ow );
        ow_enable_it ();
        ow_wait_cycles ( OW_DELAY_H );
        ow_disable_it ();
        
        /* Wait for presence */
        ow_set_input ( ow );
        ow_wait_cycles ( OW_DELAY_I );

        /* Read presence */
        read = ow_read_pin ( ow );
        ret  = !read;
        
        ow_enable_it ();
        ow_wait_cycles ( OW_DELAY_J );
    }
    
    return ret;
}


void ow_write_byte ( One_Wire* ow, uint8_t byte )
{
    uint8_t i;
     
    for ( i = ZERO; i < 8; i++ )
    {
        ow_write_bit ( ow, ( byte & 0x01 ) );
        byte >>= 1;
    }
}


uint8_t ow_read_byte ( One_Wire* ow )
{
    uint8_t i;
    uint8_t ret = ZERO;
    uint8_t bit;
    
    for ( i = 0x01; ZERO != i; i <<= 1 )
    {
        bit = ow_read_bit ( ow );
        
        if ( ZERO != bit )
        {
            ret |= i;
        }
    }
    
    return ret;
}


bool_t  ow_search ( One_Wire* ow, uint8_t* addr_buff )
{  
    bool_t  ret = FALSE;
    uint8_t id_bit_number = 1;
    uint8_t last_zero = 0;
    uint8_t rom_byte_number = 0;
    uint8_t rom_byte_mask = 1;
    uint8_t i, id_bit, cmp_id_bit, search_direction;
   
    
    if ( FALSE == ow->last_device_flag )
    {
        ret = ow_reset ( ow );
        if ( FALSE == ret )
        {
            ow_reset_search_state ( ow );
        }
    }
   
    if ( FALSE != ret )
    {
        ow_write_byte(ow, OW_CMD_SEARCH_ROM );

        do
        {
            id_bit = ow_read_bit(ow);
            cmp_id_bit = ow_read_bit(ow);
        
            if ( ( 1 == id_bit ) && ( 1 == cmp_id_bit ) )
            {
                break;
            }
            else
            {
                if ( id_bit != cmp_id_bit )
                {
                    search_direction = id_bit;
                }
                else
                {
                    if ( id_bit_number < ow->last_discrepancy )
                    {
                        search_direction = ((ow->rom[rom_byte_number]
                                         & rom_byte_mask) > 0);
                    }
                    else
                    {
                        search_direction =
                                        (id_bit_number == ow->last_discrepancy);
                    }

                    if ( 0 == search_direction )
                    {
                        last_zero = id_bit_number;

                        if ( last_zero < 9 )
                        {
                            ow->last_family_discrepancy = last_zero;
                        }
                    }
                }

                if ( 1 == search_direction )
                {
                    ow->rom[rom_byte_number] |= rom_byte_mask; 
                }
                else
                {
                    ow->rom[rom_byte_number] &= ~rom_byte_mask;
                }

                ow_write_bit(ow, search_direction);
                id_bit_number++;
                rom_byte_mask <<= 1;

                if ( 0 == rom_byte_mask )
                {
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        } while ( rom_byte_number < 8 );

        if ( id_bit_number >= 65 )
        {
            ow->last_discrepancy = last_zero;
            if (ow->last_discrepancy == 0)
            {
                ow->last_device_flag = TRUE;    
            }

            ret = TRUE;
        }
        
        if ( ( FALSE == ret ) || ( ZERO == ow->rom[0] ) )
        {
            ow->last_discrepancy = 0;
            ow->last_device_flag = FALSE;
            ow->last_family_discrepancy = 0;
            ret = FALSE;
        }
        
        for ( i = 0; i < 8; i++ )
        {
            addr_buff[i] = ow->rom[i];
        }
    }
    
   return ret;   
}


uint8_t  ow_list_all_devices ( One_Wire* ow )
{
    bool_t  b_ret;
    uint8_t i = 0;
    uint8_t no_of_dev = OW_NO_DEVICES;
    uint8_t ow_dev_rom[OW_ROM_SIZE] = {0};
    uint8_t out[SL_MAX_STRING_SIZE] = {0};
    
    bl_uart_send ( OW_UART_BASE
                 , (uint8_t*)"Devices attached to 1-Wire bus:\r\n"
                 , 33
                 );

    while ( i < OW_MAX_DEVICES )
    {
        b_ret = ow_search ( ow,  ow_dev_rom );
        
        if ( FALSE == b_ret )
        {
            break;
        }
        
        sl_sprintf_d ( out
                     , (uint8_t*)"\tDevice %d: "
                     , (int32_t)( no_of_dev + 1 )
                     , sizeof(out)
                     );
        
        bl_uart_send ( OW_UART_BASE
                     , out
                     , sl_strnlen ( out, SL_MAX_STRING_SIZE )
                     );
        
        for ( i = 0; i < OW_ROM_SIZE; i++ )
        {
            if ( 0 == i )
            {
                sl_sprintf_x ( out
                             , (uint8_t*)"%02x %02x %02x %02x %02x "
                               "%02x %02x %02x\r\n"
                             , ow_dev_rom[i]
                             , sizeof( out )
                             );
            }
            else
            {
                sl_sprintf_x ( out, out, ow_dev_rom[i], sizeof( out ) );  
            }
            
            found_rom[no_of_dev][i] = ow_dev_rom[i];
        }
        
        bl_uart_send ( OW_UART_BASE
                     , out
                     , sl_strnlen ( out, SL_MAX_STRING_SIZE )
                     );
                
        no_of_dev++;    
    }
    
    if ( OW_NO_DEVICES == no_of_dev )
    {
        bl_uart_send ( OW_UART_BASE
                     , (uint8_t*)"\tNo devices attached.\r\n"
                     , 23
                     );
    }
    
    return no_of_dev;
}


bool_t ow_match_rom ( One_Wire*   ow 
                    , uint8_t     dev_idx
                    )
{
    bool_t  ret = FALSE;
    uint8_t i;
    
    ret = ow_reset ( ow );
    
    if ( FALSE != ret )
    {
        ow_write_byte ( ow, OW_CMD_MATCH_ROM );
        
        for ( i = 0; i < OW_ROM_SIZE; i++ )
        {
            ow_write_byte ( ow, found_rom[dev_idx][i] );
        }
    }
    
    return ret;
}


bool_t ow_convert_temp ( One_Wire* ow )
{
    bool_t  ret = FALSE;
    
    ret = ow_reset ( ow );
    
    if ( FALSE != ret )
    {
        ow_write_byte ( ow, OW_CMD_SKIP_ROM );
        ow_write_byte ( ow, OW_CMD_CONVERT_TEMP );
//        ow_write_high ( ow );
//        ow_set_output ( ow );
    }
    
    return ret;
}


static void ow_write_bit ( One_Wire* ow, uint8_t bit )
{
    if ( FALSE != ( bit & 1 ) )
    {
        ow_disable_it ();
        ow_set_output ( ow );
        ow_write_low ( ow );
        ow_wait_cycles ( OW_DELAY_A );
        ow_write_high ( ow );
        ow_enable_it ();
        ow_wait_cycles ( OW_DELAY_B );
    }
    else
    {
        ow_disable_it ();
        ow_set_output ( ow );
        ow_write_low ( ow );
        ow_wait_cycles ( OW_DELAY_C );
        ow_write_high ( ow );
        ow_enable_it ();
        ow_wait_cycles ( OW_DELAY_D );
    }
}


static uint8_t ow_read_bit ( One_Wire* ow )
{
    uint8_t ret;
    
    ow_disable_it ();
    ow_set_output ( ow );
    ow_write_low ( ow );
    ow_wait_cycles ( OW_DELAY_A );
    ow_write_high ( ow ); /**< Release the bus */
    ow_wait_cycles ( OW_DELAY_E );
    ow_set_input ( ow );
    
    ret = ow_read_pin ( ow );
    ow_enable_it ();
    ow_wait_cycles ( OW_DELAY_F );
    
    return ret;
}


static HAL_Ret ow_init_gpio_clk ( GPIO_TypeDef* port )
{
    HAL_Ret ret = HAL_OK;
    
    if ( GPIOA == port )
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if ( GPIOB == port )
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if ( GPIOC == port )
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if ( GPIOD == port )
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else
    {
        ret = HAL_ERROR;
    }
    
    return ret;
}


static void ow_set_input ( One_Wire* ow )
{
    *ow->reg &= ~ow->reg_mask;
    *ow->reg |= ow->input_mask;
}


static void ow_set_output ( One_Wire* ow )
{
    *ow->reg &= ~ow->reg_mask;
    *ow->reg |= ow->output_mask;
}


static uint8_t ow_read_pin ( One_Wire* ow )
{
    return (uint8_t)( ( ow->port->IDR & ow->bitmask ) > 0 ? 1 : 0 );
}


static void ow_write_high ( One_Wire* ow )
{
    ow->port->BSRR = ow->bitmask;
}


static void ow_write_low ( One_Wire* ow )
{
    ow->port->BRR = ow->bitmask;
}


static void ow_enable_it ( void )
{
    __asm("cpsie i");
}


static void ow_disable_it ( void )
{
    __asm("cpsid i");
}


static void ow_reset_search_state ( One_Wire* ow )
{
    uint8_t i;
    
    ow->last_discrepancy        = ZERO;
    ow->last_device_flag        = FALSE;
    ow->last_family_discrepancy = ZERO;
    
    for ( i = 0; i < 8; i++ )
    {
        ow->rom[i] = ZERO;
    }
}
