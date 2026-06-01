/**
 * @file ds18b20.h
 * @brief Driver para sensor de temperatura DS18B20 por protocolo 1-Wire.
 *
 * Usa el pin RB7 como línea de datos 1-Wire.
 * La temperatura se retorna escalada (°C × 10) para evitar punto flotante.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#ifndef DS18B20_H
#define DS18B20_H

#include "config.h"

/* ------------------------------------------------------------------ */
/*  Definición del pin 1-Wire                                          */
/* ------------------------------------------------------------------ */

/** @brief Pin de datos del DS18B20. */
#define OW_PIN      PORTBbits.RB7

/** @brief Registro de dirección del pin 1-Wire. */
#define OW_TRIS     TRISBbits.TRISB7

/** @brief Registro de latch del pin 1-Wire (para escritura). */
#define OW_LAT      LATBbits.LATB7

/* ------------------------------------------------------------------ */
/*  Valor de error                                                     */
/* ------------------------------------------------------------------ */

/**
 * @brief Valor retornado cuando el sensor no responde o hay error.
 * @note  0x8000 es un valor imposible para temperatura corporal.
 */
#define DS18B20_ERROR   ((int16_t)0x8000)

/* ------------------------------------------------------------------ */
/*  Prototipos                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Inicializa el pin RB7 como entrada (estado de reposo 1-Wire).
 */
void ds18b20_init(void);

/**
 * @brief Lee la temperatura del sensor DS18B20.
 *
 * Ejecuta la secuencia completa: reset, skip ROM, convert T,
 * espera conversión y lee el scratchpad.
 *
 * @return Temperatura en décimas de grado (ej. 365 = 36.5 °C),
 *         o DS18B20_ERROR si el sensor no responde.
 */
int16_t ds18b20_leer_temperatura(void);

#endif /* DS18B20_H */
