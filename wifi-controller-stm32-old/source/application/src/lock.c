/**
  ******************************************************************************
  * @file    application/src/lock.c
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    23-Mar-2019
  * @brief   Device lock operations
  ******************************************************************************
  */

#include "lock.h"

#include "bl_flash.h"

/* Lock device value offset in flash */
#define LOCK_FLAG_OFFSET         0
/* Number of password retries offset in flash */
#define LOCK_CLI_RETRIES_OFFSET  4
/* Device is locked when status flag equals zero */
#define LOCKED_DEVICE            ((uint32_t)0)
/* Device is unlocked when status flag is different than zero */
#define UNLOCKED_DEVICE          !LOCKED_DEVICE

bool_t check_device_is_locked( void )
{
    bool_t      ret = FALSE;
    uint32_t    is_locked;

    bl_flash_read( &is_locked, sizeof(uint32_t), LOCK_FLAG_OFFSET);

    if ( LOCKED_DEVICE == is_locked )
    {
        ret = TRUE;
    }
    
    return ret;
}

HAL_Ret lock_device( void )
{
    HAL_Ret  ret;
    uint32_t lock_enable = LOCKED_DEVICE;

    ret = bl_flash_write( &lock_enable, sizeof(uint32_t), LOCK_FLAG_OFFSET );
    
    return ret;
}     

HAL_Ret lock_update_no_of_retries( uint32_t retries_left )
{
    HAL_Ret ret;

    ret = bl_flash_write( &retries_left
                        , sizeof(uint32_t)
                        , LOCK_CLI_RETRIES_OFFSET
                        );
    return ret;
}

uint32_t lock_get_no_of_retries( void )
{
    HAL_Ret  hret;
    uint32_t ret;

    hret = bl_flash_read( &ret, sizeof(uint32_t), LOCK_CLI_RETRIES_OFFSET );

    if ( HAL_OK != hret )
    {
        ret = 0xFFFFFFFF;
    }

    return ret;
}
