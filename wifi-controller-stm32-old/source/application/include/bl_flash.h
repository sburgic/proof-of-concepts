/**
  ******************************************************************************
  * @file    application/include/bl_flash.h
  * @author  Sani Sasa Burgic - sani.etf@gmail.com
  * @version V1.0.0
  * @date    21-Avg-2018
  * @brief   Flash module defines
  ******************************************************************************
 */

#ifndef BL_FLASH_H
#define BL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "stm32f1xx_hal_flash.h"

/* Base @ of Page 63, 1 Kbytes */
#define ADDR_FLASH_PAGE_63  ((uint32_t)0x0800FC00)
/* Base @ of Page 62, 1 Kbytes */
#define ADDR_FLASH_PAGE_62  ((uint32_t)0x0800F800)
/* Base @ of Page 61, 1 Kbytes */
#define ADDR_FLASH_PAGE_61  ((uint32_t)0x0800F400)
/* Base @ of Page 60, 1 Kbytes */
#define ADDR_FLASH_PAGE_60  ((uint32_t)0x0800F000)
/* Base @ of Page 59, 1 Kbytes */
#define ADDR_FLASH_PAGE_59  ((uint32_t)0x0800EC00)
/* Base @ of Page 58, 1 Kbytes */
#define ADDR_FLASH_PAGE_58  ((uint32_t)0x0800E800)
/* Base @ of Page 57, 1 Kbytes */
#define ADDR_FLASH_PAGE_57  ((uint32_t)0x0800E400)
/* Base @ of Page 56, 1 Kbytes */
#define ADDR_FLASH_PAGE_56  ((uint32_t)0x0800E000)

/* All pages erased correctly */
#define PAGE_ERASE_OK       (uint32_t)(0xFFFFFFFF)
    
#define BL_FLASH_COPY_PAGE_ADDR ADDR_FLASH_PAGE_63
#define BL_FLASH_USER_PAGE_ADDR ADDR_FLASH_PAGE_62
#define BL_FLASH_PAGE_SIZE      (1024)

HAL_Ret bl_flash_read ( void* buff, uint32_t size, uint32_t offset );
HAL_Ret bl_flash_write ( void* buff, uint32_t size, uint32_t offset );

#ifdef __cplusplus
}
#endif

#endif /* BL_FLASH_H */
