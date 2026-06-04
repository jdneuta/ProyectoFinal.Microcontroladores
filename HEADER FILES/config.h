/* 
 * File:   config.h
 * Author: MARIA DE LOS ANGELES
 *
 * Created on 21 de mayo de 2026, 17:02
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <xc.h>

#define _XTAL_FREQ 8000000UL
#pragma config PLLDIV   = 1         /**< Sin division para PLL, ya que no se usa. */
#pragma config CPUDIV   = OSC1_PLL2 /**< CPU a Fosc/1 en esta configuracion. */
#pragma config USBDIV   = 1         /**< USB no utilizado. */

#pragma config FOSC     = INTOSC_EC /**< Oscilador interno, sin cristal externo. */
#pragma config FCMEN    = OFF       /**< Monitor de falla de reloj deshabilitado. */
#pragma config IESO     = OFF       /**< Cambio interno/externo de oscilador deshabilitado. */

#pragma config PWRT     = ON        /**< Power-up Timer habilitado. */
#pragma config BOR      = ON        /**< Brown-out Reset habilitado. */
#pragma config BORV     = 3         /**< Nivel de voltaje BOR configurado en opcion 3. */
#pragma config VREGEN   = OFF       /**< Regulador USB deshabilitado. */

#pragma config WDT      = OFF       /**< Watchdog Timer deshabilitado. */
#pragma config WDTPS    = 32768     /**< Postscaler del WDT, sin efecto si WDT esta apagado. */

#pragma config CCP2MX   = ON        /**< CCP2 asignado a RC1. */
#pragma config PBADEN   = OFF       /**< PORTB inicia como digital al reset. */
#pragma config LPT1OSC  = OFF       /**< Oscilador de baja potencia de Timer1 deshabilitado. */
#pragma config MCLRE    = OFF       /**< Pin MCLR usado como entrada digital. */

#pragma config STVREN   = ON        /**< Reset por desbordamiento/subdesbordamiento de pila habilitado. */
#pragma config LVP      = OFF       /**< Programacion de bajo voltaje deshabilitada. */
#pragma config ICPRT    = OFF       /**< Puerto de depuracion/programacion deshabilitado. */
#pragma config XINST    = OFF       /**< Set extendido de instrucciones deshabilitado. */

#pragma config CP0=OFF, CP1=OFF, CP2=OFF, CP3=OFF   /**< Proteccion de codigo deshabilitada. */
#pragma config CPB=OFF, CPD=OFF                     /**< Proteccion de boot block y EEPROM deshabilitada. */
#pragma config WRT0=OFF, WRT1=OFF, WRT2=OFF, WRT3=OFF /**< Proteccion contra escritura deshabilitada. */
#pragma config WRTB=OFF, WRTC=OFF, WRTD=OFF         /**< Proteccion de escritura adicional deshabilitada. */
#pragma config EBTR0=OFF, EBTR1=OFF, EBTR2=OFF, EBTR3=OFF /**< Proteccion de lectura de tabla deshabilitada. */
#pragma config EBTRB=OFF                            /**< Proteccion de lectura de boot block deshabilitada. */

#endif
