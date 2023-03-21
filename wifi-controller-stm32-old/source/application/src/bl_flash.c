/**
  ******************************************************************************
  * @file    application/src/bl_flash.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    21-Avg-2018
  * @brief   Flash module operations
  ******************************************************************************
 */

#include "bl_flash.h"
#include "sl_handle.h"

HAL_Ret bl_flash_read ( void* buff, uint32_t size, uint32_t offset )
{
    HAL_Ret  ret = HAL_INV_HDL;
    uint32_t location = BL_FLASH_USER_PAGE_ADDR + offset;
    uint32_t i;
    uint8_t* data = (uint8_t*) buff;

    if ( HDL_IS_VALID( data ))
    {
        if (( offset + size ) > BL_FLASH_PAGE_SIZE )
        {
            ret = HAL_ERROR;
        }
        else
        {
            for ( i = 0; i < size; i++ )
            {
                data[i] = *(volatile uint8_t*)( location + i );
            }

            ret = HAL_OK;
        }
    }

    return ret;
}

HAL_Ret bl_flash_write ( void* buff, uint32_t size, uint32_t offset )
{
    HAL_Ret             ret = HAL_INV_HDL;
    Flash_Erase_Data    erase = {0};
    uint32_t            page_error;
    uint32_t            i = 0;
    uint32_t            address;

    erase.TypeErase     = FLASH_TYPEERASE_PAGES;
    erase.PageAddress   = BL_FLASH_COPY_PAGE_ADDR;
    erase.NbPages       = 1;

    if ( HDL_IS_VALID( buff ))
    {
        if (( size + offset ) > BL_FLASH_PAGE_SIZE )
        {
            ret = HAL_ERROR;
        }
        else
        {
            HAL_FLASH_Unlock ();

            /* First make a copy of existing page */
            ret = HAL_FLASHEx_Erase ( &erase, &page_error );

            if ( ( HAL_OK == ret ) && ( PAGE_ERASE_OK == page_error ) )
            {
                for ( i = 0; i < BL_FLASH_PAGE_SIZE / 4; i++ )
                {
                    address = i << 2;

                    ret |= HAL_FLASH_Program
                        ( FLASH_TYPEPROGRAM_WORD
                        , BL_FLASH_COPY_PAGE_ADDR + address
                        , *(uint32_t*)( BL_FLASH_USER_PAGE_ADDR + address )
                        );
                }
            }
            else
            {
                ret = HAL_ERROR;
            }
        }

        if ( HAL_OK == ret )
        {
            /* Erase the user page */
            erase.PageAddress = BL_FLASH_USER_PAGE_ADDR;

            ret = HAL_FLASHEx_Erase ( &erase, &page_error );

            if ( ( HAL_OK == ret ) && ( PAGE_ERASE_OK == page_error ) )
            {
                for ( i = 0; i < ( offset / 4 ); i++ )
                {
                    address = i << 2;

                    ret |= HAL_FLASH_Program
                        ( FLASH_TYPEPROGRAM_WORD
                        , BL_FLASH_USER_PAGE_ADDR + address
                        , *(uint32_t*)( BL_FLASH_COPY_PAGE_ADDR + address )
                        );
                }

                for ( i = 0; i < ( size / 4 ); i++ )
                {
                    address = offset + ( i << 2 );

                    ret |= HAL_FLASH_Program
                            ( FLASH_TYPEPROGRAM_WORD
                            , BL_FLASH_USER_PAGE_ADDR + address
                            , *(uint32_t*)( buff + ( i << 2 ))
                            );
                }

                for ( i = 0;
                      i < (( BL_FLASH_PAGE_SIZE - size - offset ) / 4 );
                      i++
                    )
                {
                    address = ( offset + size + ( i << 2 ));

                    ret |= HAL_FLASH_Program
                            ( FLASH_TYPEPROGRAM_WORD
                            , BL_FLASH_USER_PAGE_ADDR + address
                            , *(uint32_t*)( BL_FLASH_COPY_PAGE_ADDR + address )
                            );
                }
            }
        }

        HAL_FLASH_Lock ();
    }

    return ret;
}
