/**
 * @file config.h
 * @brief Configuración del PIC18F4550 y constantes globales del proyecto.
 *
 * Define los bits de configuración del microcontrolador, la frecuencia
 * de operación y las constantes de umbrales fisiológicos usadas en todo
 * el sistema.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

/* ------------------------------------------------------------------ */
/*  Bits de configuración PIC18F4550, oscilador interno 8 MHz        */
/* ------------------------------------------------------------------ */

#pragma config PLLDIV   = 1
#pragma config CPUDIV   = OSC1_PLL2
#pragma config USBDIV   = 1
#pragma config FOSC     = INTOSC_HS
#pragma config FCMEN    = OFF
#pragma config IESO     = OFF
#pragma config PWRT     = ON
#pragma config BOR      = ON
#pragma config BORV     = 3
#pragma config WDT      = OFF
#pragma config WDTPS    = 32768
#pragma config CCP2MX   = ON
#pragma config PBADEN   = OFF
#pragma config LPT1OSC  = OFF
#pragma config MCLRE    = ON
#pragma config STVREN   = ON
#pragma config LVP      = OFF
#pragma config ICPRT    = OFF
#pragma config XINST    = OFF
#pragma config CP0      = OFF
#pragma config CP1      = OFF
#pragma config CP2      = OFF
#pragma config CP3      = OFF
#pragma config CPB      = OFF
#pragma config CPD      = OFF
#pragma config WRT0     = OFF
#pragma config WRT1     = OFF
#pragma config WRT2     = OFF
#pragma config WRT3     = OFF
#pragma config WRTB     = OFF
#pragma config WRTC     = OFF
#pragma config WRTD     = OFF
#pragma config EBTR0    = OFF
#pragma config EBTR1    = OFF
#pragma config EBTR2    = OFF
#pragma config EBTR3    = OFF
#pragma config EBTRB    = OFF

/* ------------------------------------------------------------------ */
/*  Frecuencia de operación                                            */
/* ------------------------------------------------------------------ */

/** @brief Frecuencia del CPU en Hz. Requerida por __delay_ms / __delay_us. */
#define _XTAL_FREQ  8000000UL

/* ------------------------------------------------------------------ */
/*  Umbrales fisiológicos                                              */
/* ------------------------------------------------------------------ */

/** @brief Frecuencia cardíaca mínima normal (BPM). */
#define FC_MIN      60u

/** @brief Frecuencia cardíaca máxima normal (BPM). */
#define FC_MAX      100u

/**
 * @brief Temperatura mínima normal escalada (°C × 10).
 * @note  Se usa entero para evitar punto flotante: 360 = 36.0 °C
 */
#define TEMP_MIN    360

/**
 * @brief Temperatura máxima normal escalada (°C × 10).
 * @note  375 = 37.5 °C
 */
#define TEMP_MAX    375

/* ------------------------------------------------------------------ */
/*  Parámetros de tiempo                                               */
/* ------------------------------------------------------------------ */

/** @brief Periodo entre lecturas de sensores (ms). */
#define PERIODO_MS  1000u

/** @brief Tiempo de espera en modo standby (ms). */
#define STANDBY_MS  200u

#endif /* CONFIG_H */
