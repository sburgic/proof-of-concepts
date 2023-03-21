/**
 ** Name
 **   flash.c
 **
 ** Purpose
 **   Flash routines
 **
 ** Revision
 **   28-Aug-2020 (SSB) [] Initial
 **/

#include "flash.h"

status_t flash_read( void* buff, uint32_t size, uint32_t offset )
{
    status_t ret      = STATUS_ERROR;
    uint32_t location = FLASH_USER_PAGE_ADDR + offset;
    uint32_t i;
    uint8_t* data     = (uint8_t*) buff;

    if ( NULL != data )
    {
        if (( offset + size ) <= FLASH_PAGE_SIZE )
        {
            for ( i = 0; i < size; i++ )
            {
                data[i] = *(volatile uint8_t*)( location + i );
            }

            ret = STATUS_OK;
        }
    }

    return ret;
}

status_t flash_write( void* buff, uint32_t size, uint32_t offset )
{
    status_t               ret  = STATUS_OK;
    HAL_StatusTypeDef      hret = HAL_ERROR;
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t               page_error;
    uint32_t               i = 0;
    uint32_t               address;

    erase.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = FLASH_COPY_PAGE_ADDR;
    erase.NbPages     = 1;

    if ( NULL != buff )
    {
        if (( size + offset ) <= FLASH_PAGE_SIZE )
        {
            HAL_FLASH_Unlock();

            /* First make a copy of existing page */
            hret = HAL_FLASHEx_Erase( &erase, &page_error );

            if (( HAL_OK == hret ) && ( FLASH_PAGE_ERASE_OK == page_error ))
            {
                for ( i = 0; i < FLASH_PAGE_SIZE / 4; i++ )
                {
                    address = i << 2;

                    hret |= HAL_FLASH_Program
                                ( FLASH_TYPEPROGRAM_WORD
                                , FLASH_COPY_PAGE_ADDR + address
                                , *(uint32_t*)( FLASH_USER_PAGE_ADDR + address )
                                );
                }
            }
            else
            {
                hret = HAL_ERROR;
            }
        }

        if ( HAL_OK == hret )
        {
            /* Erase the user page */
            erase.PageAddress = FLASH_USER_PAGE_ADDR;

            hret = HAL_FLASHEx_Erase( &erase, &page_error );

            if (( HAL_OK == hret ) && ( FLASH_PAGE_ERASE_OK == page_error ))
            {
                for ( i = 0; i < ( offset / 4 ); i++ )
                {
                    address = i << 2;

                    hret |= HAL_FLASH_Program
                                ( FLASH_TYPEPROGRAM_WORD
                                , FLASH_USER_PAGE_ADDR + address
                                , *(uint32_t*)( FLASH_COPY_PAGE_ADDR + address )
                                );
                }

                for ( i = 0; i < ( size / 4 ); i++ )
                {
                    address = offset + ( i << 2 );

                    hret |= HAL_FLASH_Program
                                ( FLASH_TYPEPROGRAM_WORD
                                , FLASH_USER_PAGE_ADDR + address
                                , *(uint32_t*)( buff + ( i << 2 ))
                                );
                }

                for ( i = 0;
                      i < (( FLASH_PAGE_SIZE - size - offset ) / 4 );
                      i++
                    )
                {
                    address = ( offset + size + ( i << 2 ));

                    hret |= HAL_FLASH_Program
                                ( FLASH_TYPEPROGRAM_WORD
                                , FLASH_USER_PAGE_ADDR + address
                                , *(uint32_t*)( FLASH_COPY_PAGE_ADDR + address )
                                );
                }
            }
        }

        HAL_FLASH_Lock();
    }

    if ( HAL_OK != hret )
    {
        ret = STATUS_ERROR;
    }

    return ret;
}
