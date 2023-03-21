#ifndef __DHT11_H__
#define __DHT11_H__

#include "system.h"

#define DHT11_GPIO_SET_INPUT()  do { P1DIR &= ~BIT7; } while (0)
#define DHT11_GPIO_SET_OUTPUT() do { P1DIR |= BIT7; } while (0)
#define DHT11_GPIO_WRITE_LOW()  do { P1OUT &= ~BIT7; } while (0)
#define DHT11_GPIO_WRITE_HIGH() do { P1OUT |= BIT7; } while (0)
#define DHT11_GPIO_READ_PIN(x) \
                       do { x = (uint8_t)( 0 != ( P1IN & BIT7 )) > 0 ? 1 : 0; }\
                       while (0)

typedef struct
{
    uint8_t hum_int;
    uint8_t hum_dec;
    uint8_t temp_int;
    uint8_t temp_dec;
} Dht11_t;

uint8_t dht11_read( Dht11_t* dht_data );

#endif /* __DHT11_H__ */
