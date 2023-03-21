#include "dht11.h"

#define DHT11_CYCLES_80US   ((uint8_t)110)
#define DHT11_CYCLES_26US   ((uint8_t)80)

static inline void dht11_set_input( void )
{
    DHT11_GPIO_SET_INPUT();
}

static inline void dht11_set_output( void )
{
    DHT11_GPIO_SET_OUTPUT();
}

static inline void dht11_write_low( void )
{
    DHT11_GPIO_WRITE_LOW();
}

static inline void dht11_write_high( void )
{
    DHT11_GPIO_WRITE_HIGH();
}

static inline uint8_t dht11_read_pin( void )
{
    uint8_t state;

    DHT11_GPIO_READ_PIN( state );

    return state;
}

uint8_t dht11_read( Dht11_t* dht_data )
{
    uint8_t  ret     = 0xFF;
    uint16_t elapsed = 0;
    uint8_t  crc     = 0;
    uint8_t  tmp     = 0;
    uint8_t  i;

    /* Reset data */
    dht_data->hum_int  = 0;
    dht_data->hum_dec  = 0;
    dht_data->temp_int = 0;
    dht_data->temp_dec = 0;

    dht11_set_output();
    dht11_write_low();
    wait( 20000 ); /* Delay has to take at least 18 ms */
    dht11_write_high();
    dht11_set_input();
    while( 0 != dht11_read_pin());

    /* DHT11 keeps low for 80 us */
    do
    {
        elapsed++;
    } while( 0 == dht11_read_pin());

    if ( elapsed > DHT11_CYCLES_80US )
    {
        elapsed = 0;

        /* DHT11 keeps high for 80 us */
        do
        {
            elapsed++;
        } while( 0 != dht11_read_pin());

        if ( elapsed > DHT11_CYCLES_80US )
        {
            /*Get humidity integer part */
            for ( i = 0; i < 8; i++ )
            {
                while( 0 == dht11_read_pin()); /* Skip the first 50 us */
                elapsed = 0;

                do
                {
                    elapsed++;
                } while( 0 != dht11_read_pin());

                if ( elapsed > DHT11_CYCLES_26US )
                {
                    dht_data->hum_int |= 0x01;

                }
                else
                {
                    dht_data->hum_int &= ~0x01;
                }

                dht_data->hum_int <<= 1;
            }

            /*Get humidity decimal part */
            for ( i = 0; i < 8; i++ )
            {
                while( 0 == dht11_read_pin()); /* Skip the first 50 us */
                elapsed = 0;

                do
                {
                    elapsed++;
                } while( 0 != dht11_read_pin());

                if ( elapsed > DHT11_CYCLES_26US )
                {
                    dht_data->hum_dec |= 0x01;

                }
                else
                {
                    dht_data->hum_dec &= ~0x01;
                }

                dht_data->hum_dec <<= 1;
            }

            /*Get temperature integer part */
            for ( i = 0; i < 8; i++ )
            {
                while( 0 == dht11_read_pin()); /* Skip the first 50 us */
                elapsed = 0;

                do
                {
                    elapsed++;
                } while( 0 != dht11_read_pin());

                if ( elapsed > DHT11_CYCLES_26US )
                {
                    dht_data->temp_int |= 0x01;

                }
                else
                {
                    dht_data->temp_int &= ~0x01;
                }

                dht_data->temp_int <<= 1;
            }

            /*Get temperature decimal part */
            for ( i = 0; i < 8; i++ )
            {
                while( 0 == dht11_read_pin()); /* Skip the first 50 us */
                elapsed = 0;

                do
                {
                    elapsed++;
                } while( 0 != dht11_read_pin());

                if ( elapsed > DHT11_CYCLES_26US )
                {
                    dht_data->temp_dec |= 0x01;

                }
                else
                {
                    dht_data->temp_dec &= ~0x01;
                }

                dht_data->temp_dec <<= 1;
            }
        }

        /*Get CRC */
        for ( i = 0; i < 8; i++ )
        {
            while( 0 == dht11_read_pin()); /* Skip the first 50 us */
            elapsed = 0;

            do
            {
                elapsed++;
            } while( 0 != dht11_read_pin());

            if ( elapsed > DHT11_CYCLES_26US )
            {
                crc |= 0x01;

            }
            else
            {
                crc &= ~0x01;
            }

            crc <<= 1;
        }

        tmp = dht_data->hum_int
            + dht_data->hum_dec
            + dht_data->temp_int
            + dht_data->temp_dec;

        if ( crc == tmp )
        {
            ret = 0;
        }
    }

    return ret;
}
