/* 
 * File:   config.h
 * Author: María De Los Ángeles Castillo 
 *
 * Created on 18 de abril de 2026, 01:51 PM
 */

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// CONFIG BITS -- Oscilador interno 8MHz
// ============================================================

// CONFIG1L
#pragma config PLLDIV   = 1         // Sin división (PLL no se usa)
#pragma config CPUDIV   = OSC1_PLL2 // CPU corre a Fosc/1
#pragma config USBDIV   = 1         // USB no se usa

// CONFIG1H
#pragma config FOSC     = INTOSC_EC // Oscilador interno, sin cristal 
#pragma config FCMEN    = OFF
#pragma config IESO     = OFF

// CONFIG2L
#pragma config PWRT     = ON        // Power-up timer ON
#pragma config BOR      = ON        // Brown-out reset ON
#pragma config BORV     = 3
#pragma config VREGEN   = OFF

// CONFIG2H
#pragma config WDT      = OFF
#pragma config WDTPS    = 32768

// CONFIG3H
#pragma config CCP2MX   = ON        // CCP2 en RC1
#pragma config PBADEN   = OFF       // PORTB digital al reset
#pragma config LPT1OSC  = OFF
#pragma config MCLRE    = OFF

// CONFIG4L
#pragma config STVREN   = ON
#pragma config LVP      = OFF
#pragma config ICPRT    = OFF
#pragma config XINST    = OFF

// CONFIG5L/5H/6L/6H/7L/7H
#pragma config CP0=OFF, CP1=OFF, CP2=OFF, CP3=OFF
#pragma config CPB=OFF, CPD=OFF
#pragma config WRT0=OFF, WRT1=OFF, WRT2=OFF, WRT3=OFF
#pragma config WRTB=OFF, WRTC=OFF, WRTD=OFF
#pragma config EBTR0=OFF, EBTR1=OFF, EBTR2=OFF, EBTR3=OFF
#pragma config EBTRB=OFF

#ifndef CONFIG_H
#define CONFIG_H

#include <xc.h>

#define _XTAL_FREQ 8000000UL

/*static inline void OSC_Init(void)
{
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF0 = 1;

    OSCCONbits.SCS1 = 1;
    OSCCONbits.SCS0 = 0;

    while(!OSCCONbits.IOFS)
    {
        ;
    }
}*/

#endif
