#ifndef __MQ3_H__
#define __MQ3_H__

#include "system.h"

#define MQ3_AN_IN_PIN ADCPCTL1  /* P1.1 */
#define MQ3_AN_IN_CH  ADCINCH_1 /* A1 */

#define MQ3_PB_START_INIT()  do { P2DIR &= ~BIT2; \
                                  P2REN |= BIT2;  \
                                  P2OUT |= BIT2; } while (0)
#define MQ3_PB_START_READ(x) \
                       do { x = (uint8_t)( 0 != ( P2IN & BIT2 )) > 0 ? 1 : 0; }\
                       while (0)

#define MQ3_PB_AUX_INIT()  do { P2DIR &= ~BIT3; \
                                P2REN |= BIT3;  \
                                P2OUT |= BIT3; } while (0)
#define MQ3_PB_AUX_READ(x) \
                       do { x = (uint8_t)( 0 != ( P2IN & BIT3 )) > 0 ? 1 : 0; }\
                       while (0)

#define MQ3_ERR_VALUE ((uint16_t)0xFFFF)
#define MQ3_HEAT_TIME (90) /* Heating time in seconds */
#define MQ3_SAMP_TIME (3)  /* Sampling time in seconds */

#define MQ3_STATUS_SOBER  ((uint8_t*)"SOBER")
#define MQ3_STATUS_LEGAL  ((uint8_t*)"LEGAL")
#define MQ3_STATUS_DRUNK  ((uint8_t*)"DRUNK")

#ifdef CANADOOO
#define MQ3_STATUS_WINNER ((uint8_t*)"CANADOOO")
#else
#define MQ3_STATUS_WINNER ((uint8_t*)"WINNER")
#endif

#define MQ3_HIGH_SCORE_FRAM_ADDR (0x1800)

typedef enum
{
    MQ3_LEVEL_SOBER = 0,
    MQ3_LEVEL_LEGAL,
    MQ3_LEVEL_DRUNK,
    MQ3_LEVEL_WINNER
} Mq3_Level_t;

void mq3_init( void );
uint16_t mq3_get_value( void );

#endif /* __MQ3_H__ */
