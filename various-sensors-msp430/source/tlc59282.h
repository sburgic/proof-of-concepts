#ifndef __TLC59282_H__
#define __TLC59282_H__

#include <stdint.h>

/* Number of connected segments */
#define TLC_NO_OF_SEG ((uint8_t)4)

/* Initialize TLC59282 */
void tlc59282_init( void );
/* Send uint32_t data */
void tlc59282_send_uint32( uint32_t data );
/* Send uint32_t data without latch */
void tlc59282_send_uint32_nl( uint32_t data );
/* Send uint16_t data */
void tlc59282_send_uint16( uint16_t data );
/* Test all segments */
void tlc59282_led_test( void );
/* Send string */
void tlc59282_led_puts( uint8_t* str, uint8_t size );
/* Clear display - all off */
void tlc59282_clear( void );

#endif /* __TLC59282_H__ */
