#define THERMOMETER

#ifdef ALCOTEST
    #include "lcd2wire.h"
    #include "mq3.h"
#endif

#ifdef THERMOMETER
    #include "ds18b20.h"
    #include "tlc59282.h"
#endif

#include "system.h"

#include <stdio.h>

int main(void)
{
#ifdef ALCOTEST
    uint32_t    i;
    uint8_t     lcd_out[LCD_TOTAL_COLUMNS] = {0};
    uint64_t    mq3_val;
    Mq3_Level_t mq3_level;
    uint8_t     mq3_pb_state = 1;
    uint8_t     mq3_aux_state;
    bool_t      start = FALSE;
    uint8_t*    ptr;
    uint16_t    raw;
    uint16_t    offset;
    uint64_t    collected_samples = 0;
    bool_t      done;
    bool_t      reset_hs = FALSE;
    uint64_t    time_start;
    uint64_t    time_act;
    uint64_t    time_tmp_1;
    uint64_t    time_tmp_2;
    bool_t      time_1s_flag;
    uint16_t*   mq3_hs_ptr;
#endif

#ifdef THERMOMETER
    Ds_Hdl_t* ds;
    uint8_t   ds_out[TLC_NO_OF_SEG + 1] = {0};
#endif

    system_init();

#ifdef ALCOTEST
    lcd_init();
    mq3_init();

    mq3_hs_ptr = (uint16_t*) MQ3_HIGH_SCORE_FRAM_ADDR;

    MQ3_PB_AUX_READ( mq3_aux_state );

    if ( 0 == mq3_aux_state )
    {
        mq3_pb_state = 0;
    }

    lcd_puts_xy( "Alcotest", 4, 0 );
    lcd_puts_xy( "SW ver.1.1", 3, 1 );
    wait( 2000000 );
    lcd_clear();

    MQ3_PB_AUX_READ( mq3_pb_state );

    if ( 0 == mq3_pb_state )
    {
        reset_hs = TRUE;
    }

    if (( 0xFFFF == *mq3_hs_ptr ) || ( FALSE != reset_hs ))
    {
        SYSCFG0 = FRWPPW | PFWP;
        *mq3_hs_ptr = 0;
        SYSCFG0 = FRWPPW | PFWP | DFWP;
    }

    sprintf((char*) lcd_out, "Highscore:%d", *mq3_hs_ptr );
    lcd_puts_xy_cl( lcd_out, 0, 0 );

    for ( i = MQ3_HEAT_TIME; i > 0; i-- )
    {
        sprintf((char*) lcd_out, "Heating... %ds", i );
        lcd_puts_xy_cl( lcd_out, 0, 1 );
        wait( 1100000 );
    }

    lcd_clear();
    lcd_puts_xy( "Ready...", 0, 0 );

    for (;;)
    {
        MQ3_PB_START_READ( mq3_pb_state );

        if ( 0 == mq3_pb_state )
        {
            wait( 75000 );
            MQ3_PB_START_READ( mq3_pb_state );

            if ( 0 == mq3_pb_state )
            {
                do
                {
                    MQ3_PB_START_READ( mq3_pb_state );
                } while ( 0 == mq3_pb_state );

                start = TRUE;
            }
        }

        if ( FALSE != start )
        {
            mq3_val = 0;
            lcd_clear();

            for ( i = 0; i < 100; i++ )
            {
                raw = mq3_get_value();
                mq3_val += ( raw * 2 );
                wait( 1000 );
            }

            mq3_val /= 100;

            offset            = mq3_val;
            done              = FALSE;
            time_1s_flag      = FALSE;
            i                 = MQ3_SAMP_TIME;
            mq3_val           = 0;
            collected_samples = 0;
            time_start        = get_tick();

            sprintf((char*) lcd_out, "Go... %ds", i );
            lcd_puts_xy_cl( lcd_out, 0, 1 );
            i--;

            do
            {
                raw = mq3_get_value();
                mq3_val += ( raw * 2 );
                collected_samples++;

                time_act = get_tick();

                if (( time_act - time_start ) >= ( MQ3_SAMP_TIME * 1000000 ))
                {
                    done = TRUE;
                }
                else
                {
                    if ( FALSE == time_1s_flag )
                    {
                        time_tmp_1   = get_tick();
                        time_1s_flag = TRUE;
                    }
                    else
                    {
                        time_tmp_2 = get_tick();

                        if (( time_tmp_2 - time_tmp_1 ) > 1000000 )
                        {
                            sprintf((char*) lcd_out, "Go... %ds", i );
                            lcd_puts_xy_cl( lcd_out, 0, 1 );
                            i--;
                            time_1s_flag = FALSE;
                        }
                    }
                }

                wait( 1000 );
            }
            while ( FALSE == done );

            mq3_val /= collected_samples;

            if ( mq3_val >= offset )
            {
                mq3_val -= offset;
            }
            else
            {
                mq3_val = ( offset - mq3_val );
            }

            if ( mq3_val <= 120 )
            {
                mq3_level = MQ3_LEVEL_SOBER;
            }
            else if (( mq3_val > 120 ) && ( mq3_val <= 400 ))
            {
                mq3_level = MQ3_LEVEL_LEGAL;
            }
            else if (( mq3_val > 400 ) && ( mq3_val <= 700 ))
            {
                mq3_level = MQ3_LEVEL_DRUNK;
            }
            else
            {
                mq3_level = MQ3_LEVEL_WINNER;
            }

            switch ( mq3_level )
            {
                case MQ3_LEVEL_SOBER:
                    ptr = MQ3_STATUS_SOBER;
                    break;
                case MQ3_LEVEL_LEGAL:
                    ptr = MQ3_STATUS_LEGAL;
                    break;
                case MQ3_LEVEL_DRUNK:
                    ptr = MQ3_STATUS_DRUNK;
                    break;
                case MQ3_LEVEL_WINNER:
                    ptr = MQ3_STATUS_WINNER;
                    break;
            }

            if ( mq3_val > *mq3_hs_ptr )
            {
                SYSCFG0 = FRWPPW | PFWP;
                *mq3_hs_ptr = mq3_val;
                SYSCFG0 = FRWPPW | PFWP | DFWP;

                sprintf((char*) lcd_out, "%d:%s", (uint16_t) mq3_val, ptr );
                lcd_puts_xy_cl( lcd_out, 0, 0 );
                lcd_puts_xy_cl((uint8_t*)"New highscore!", 0, 1 );
                wait( 3000000 );
            }
            else
            {
                sprintf((char*) lcd_out, "%d:%s", (uint16_t) mq3_val, ptr );
                lcd_puts_xy_cl( lcd_out, 0, 0 );
            }

            for ( i = MQ3_HEAT_TIME; i > 0; i-- )
            {
                sprintf((char*) lcd_out, "New start in %ds", i );
                lcd_puts_xy_cl( lcd_out, 0, 1 );
                wait( 1100000 );
            }

            lcd_clear();
            lcd_puts_xy( "Ready...", 0, 0 );

            start = FALSE;
        }
    }
#endif

#ifdef THERMOMETER
    ds18b20_init();
    tlc59282_init();

    tlc59282_clear();
    tlc59282_led_test();

    ds = ds18b20_get_hdl();

    for(;;)
    {
        ds18b20_update();

        if ( DS_SENSOR_ERROR != ds->last_temperature[0] )
        {
#ifdef THERMOMETER2DIGITS
            if (( ds->last_temperature[0] >> 4 ) > 99 )
            {
                tlc59282_led_puts((uint8_t*) "99.", 3);
            }
            else if (( ds->last_temperature[0] >> 4 ) < -9 )
            {
                tlc59282_led_puts((uint8_t*) "-9.", 3 );
            }
            else if ((( ds->last_temperature[0] >> 4 ) <= 9 )
                  && (( ds->last_temperature[0] >> 4 ) > 0 ))
            {
                ds_out[0] = '0';
                ds_out[1] = (( ds->last_temperature[0] >> 4 ) + 48 );

                tlc59282_led_puts( ds_out, 2 );
            }
            else if (( 0 == (( ~ds->last_temperature[0] + 1 ) >> 4 )
                  && ( 0 != ( ds->last_temperature[0] & 0x8000 )))
                  || ( 0 == ( ds->last_temperature[0] >> 4 )))
            {
                ds_out[0] = '0';
                ds_out[1] = '0';

                tlc59282_led_puts( ds_out, 2 );
            }
            else
            {
                ds18b20_temp_to_string( ds_out, ds->last_temperature[0] );
                tlc59282_led_puts( ds_out
                                 , utils_strnlen( ds_out, TLC_NO_OF_SEG )
                                 );
            }
#else
            ds18b20_temp_to_string( ds_out, ds->last_temperature[0] );

            if ((( ds->last_temperature[0] >> 4 ) > 99 )
             || (( ds->last_temperature[0] >> 4 ) < -9  ))
            {
                tlc59282_led_puts( ds_out, TLC_NO_OF_SEG + 1 );
            }
            else
            {
                tlc59282_led_puts( ds_out
                                 , utils_strnlen( ds_out, TLC_NO_OF_SEG )
                                 );
            }
#endif
        }
        else
        {
#ifdef THERMOMETER2DIGITS
            tlc59282_led_puts((uint8_t*) "Er.", 3 );
#else
            tlc59282_led_puts((uint8_t*) "Err.", 4 );
#endif
        }

        wait( 1000000 );
    }
#endif

    return 0;
}
