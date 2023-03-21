#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <msp430.h>
#include <stdint.h>

#ifndef FALSE
    #define FALSE (0)
    #define TRUE !(FALSE)
#endif

#ifndef NULL
    #define NULL ((void*)0)
#endif

typedef uint8_t bool_t;

void system_init( void );
void wait( int64_t us );
uint64_t get_tick( void );
void timer2_A2_irq_hdl( void );
uint32_t utils_strnlen( const uint8_t* s, uint32_t len );

#endif /* __SYSTEM_H__ */
