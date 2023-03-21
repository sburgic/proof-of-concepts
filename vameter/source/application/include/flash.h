/**
 ** Name
 **   flash.h
 **
 ** Purpose
 **   Flash definitions
 **
 ** Revision
 **   28-Aug-2020 (SSB) [] Initial
 **/

#ifndef __FLASH_H__
#define __FLASH_H__

#include "ptypes.h"

#include <stm32f1xx_hal.h>

#define FLASH_ADDR_PAGE_127 ((uint32_t)0x0801FC00)
#define FLASH_ADDR_PAGE_126 ((uint32_t)0x0801F800)
#define FLASH_ADDR_PAGE_125 ((uint32_t)0x0801F400)
#define FLASH_ADDR_PAGE_124 ((uint32_t)0x0801F000)
#define FLASH_ADDR_PAGE_123 ((uint32_t)0x0801EC00)
#define FLASH_ADDR_PAGE_122 ((uint32_t)0x0801E800)
#define FLASH_ADDR_PAGE_121 ((uint32_t)0x0801E400)
#define FLASH_ADDR_PAGE_120 ((uint32_t)0x0801E000)
#define FLASH_ADDR_PAGE_63  ((uint32_t)0x0800FC00)
#define FLASH_ADDR_PAGE_62  ((uint32_t)0x0800F800)
#define FLASH_ADDR_PAGE_61  ((uint32_t)0x0800F400)
#define FLASH_ADDR_PAGE_60  ((uint32_t)0x0800F000)
#define FLASH_ADDR_PAGE_59  ((uint32_t)0x0800EC00)
#define FLASH_ADDR_PAGE_58  ((uint32_t)0x0800E800)
#define FLASH_ADDR_PAGE_57  ((uint32_t)0x0800E400)
#define FLASH_ADDR_PAGE_56  ((uint32_t)0x0800E000)

#define FLASH_PAGE_ERASE_OK (uint32_t)(0xFFFFFFFF)

#ifdef STM32F100xB
    #define FLASH_COPY_PAGE_ADDR FLASH_ADDR_PAGE_63
    #define FLASH_USER_PAGE_ADDR FLASH_ADDR_PAGE_62
#else
    #define FLASH_COPY_PAGE_ADDR FLASH_ADDR_PAGE_127
    #define FLASH_USER_PAGE_ADDR FLASH_ADDR_PAGE_126
#endif

status_t flash_read( void* buff, uint32_t size, uint32_t offset );
status_t flash_write( void* buff, uint32_t size, uint32_t offset );

#endif /* __FLASH_H__ */
