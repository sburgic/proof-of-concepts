/**
  ******************************************************************************
  * @file    application/include/lock.h
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    20-Oct-2017
  * @brief   Device lock operations
  ******************************************************************************
  */

#ifndef LOCK_H
#define LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
    
/* No of max password retries */
#define LOCK_CLI_MAX_RETRY ((uint32_t)3)

/* Check if device is locked */
bool_t check_device_is_locked ( void );
/* Lock device */
HAL_Ret lock_device ( void );
/* Update number of login retries */
HAL_Ret lock_update_no_of_retries( uint32_t retries_left );
/* Get number of login retries */
uint32_t lock_get_no_of_retries( void );

#ifdef __cplusplus
}
#endif

#endif /* LOCK_H */
