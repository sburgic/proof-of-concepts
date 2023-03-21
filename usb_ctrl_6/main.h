/**
 ** Name
 **   main.c
 **
 ** Purpose
 **   Main application definitions
 **
 ** Revision
 **   06-Feb-2021 (SSB) [] Initial
 **/

#ifndef __MAIN_H__
#define __MAIN_H__

#ifndef _XTAL_FREQ
  #define _XTAL_FREQ 4000000
#endif

#include <htc.h>
#include <pic.h>

__CONFIG(FOSC_XT   \
       & WDTE_OFF  \
       & PWRTE_OFF \
       & MCLRE_ON  \
       & CP_OFF    \
       & CPD_OFF   \
       & BOREN_OFF \
       & LVP_OFF);

__CONFIG(FOSC_XT    \
       & WRT_OFF    \
       & PLLEN_OFF  \
       & STVREN_OFF \
       & BORV_LO);

#endif /* __MAIN_H__ */
