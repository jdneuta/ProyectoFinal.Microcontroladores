/**
 * @file leds.c
 * @brief Implementación del módulo de control de LEDs.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#include "leds.h"

/**
 * @brief Inicializa los pines RA1 - RA5 como salidas digitales y los apaga.
 *
 * Configura el registro TRISA para que los pines de los LEDs sean salidas.
 * RA0 no se toca para no interferir con otros usos del puerto.
 */
void leds_init(void) {
    TRISAbits.TRISA1 = 0;   // LED_ENCENDIDO  : salida
    TRISAbits.TRISA2 = 0;   // LED_PREPARANDO : salida
    TRISAbits.TRISA3 = 0;   // LED_EN_ESPERA  : salida
    TRISAbits.TRISA4 = 0;   // LED_FUNCIONAL  : salida
    TRISAbits.TRISA5 = 0;   // LED_ALARMA     : salida

    leds_apagar_todos();
}

/**
 * @brief Apaga todos los LEDs de estado simultáneamente.
 */
void leds_apagar_todos(void) {
    LED_ENCENDIDO  = 0;
    LED_PREPARANDO = 0;
    LED_EN_ESPERA  = 0;
    LED_FUNCIONAL  = 0;
    LED_ALARMA     = 0;
}

/**
 * @brief Parpadea el LED de alarma un número determinado de veces.
 *
 * Cada ciclo enciende el LED 150 ms y lo apaga 150 ms.
 *
 * @param ciclos Número de parpadeos completos a realizar.
 */
void leds_parpadeo_alarma(uint8_t ciclos) {
    for (uint8_t i = 0; i < ciclos; i++) {
        LED_ALARMA = 1;
        __delay_ms(150);
        LED_ALARMA = 0;
        __delay_ms(150);
    }
}
