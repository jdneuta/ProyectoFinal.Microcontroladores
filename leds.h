/**
 * @file leds.h
 * @brief Control de LEDs de estado del sistema.
 *
 * Define los pines y funciones para manejar los 5 LEDs de estado:
 * Encendido (RA1), Preparando (RA2), En espera (RA3),
 * Funcional (RA4) y Alarma (RA5).
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#ifndef LEDS_H
#define LEDS_H

#include "config.h"

/* ------------------------------------------------------------------ */
/*  Definición de pines                                                */
/* ------------------------------------------------------------------ */

/** @brief LED que indica que el sistema está encendido. */
#define LED_ENCENDIDO   LATAbits.LATA1

/** @brief LED que indica que el sistema está inicializando. */
#define LED_PREPARANDO  LATAbits.LATA2

/** @brief LED que indica que el sistema espera un dedo sobre el sensor. */
#define LED_EN_ESPERA   LATAbits.LATA3

/** @brief LED que indica que el sistema está midiendo activamente. */
#define LED_FUNCIONAL   LATAbits.LATA4

/** @brief LED que indica condición de alarma. */
#define LED_ALARMA      LATAbits.LATA5

/* ------------------------------------------------------------------ */
/*  Prototipos                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Inicializa los pines de los LEDs como salidas y los apaga todos.
 */
void leds_init(void);

/**
 * @brief Apaga todos los LEDs de estado simultáneamente.
 */
void leds_apagar_todos(void);

/**
 * @brief Activa la secuencia de parpadeo del LED de alarma.
 * @param ciclos Número de parpadeos a realizar.
 */
void leds_parpadeo_alarma(uint8_t ciclos);

#endif /* LEDS_H */
