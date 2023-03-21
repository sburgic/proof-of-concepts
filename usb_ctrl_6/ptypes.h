/**
 ** Name
 **   ptypes.h
 **
 ** Purpose
 **   Primitive types
 **
 ** Revision
 **   06-Feb-2021 (SSB) [] Initial
 **/

#ifndef __PTYPES_H__
#define __PTYPES_H__

#include <stdint.h>

#ifndef NULL
  #define NULL ((void*)0)
#endif

#ifndef FALSE
  #define FALSE (0)
  #define TRUE  !(FALSE)
#endif

typedef enum
{
    STATUS_OK = 0,
    STATUS_ERROR
} status_t;

typedef uint8_t bool_t;

#endif /* __PTYPES_H__ */
