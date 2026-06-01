/**
 * @file alarma.h
 * @brief Módulo de evaluación de umbrales y activación de alarma.
 *
 * Compara los valores medidos de FC y temperatura con los umbrales
 * definidos en config.h y coordina la alarma visual y el reporte UART.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#ifndef ALARMA_H
#define ALARMA_H

#include "config.h"
#include "leds.h"
#include "uart.h"

/* ------------------------------------------------------------------ */
/*  Prototipos                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Evalúa si los valores de FC y temperatura están fuera de rango.
 *
 * @param fc       Frecuencia cardíaca en BPM.
 * @param temp_x10 Temperatura escalada (°C × 10).
 * @return true si alguno de los valores supera los umbrales definidos.
 */
bool alarma_evaluar(uint8_t fc, int16_t temp_x10);

/**
 * @brief Activa la alarma visual y envía el reporte de alerta por UART.
 *
 * @param fc       Frecuencia cardíaca medida.
 * @param temp_x10 Temperatura medida escalada.
 */
void alarma_activar(uint8_t fc, int16_t temp_x10);

/**
 * @brief Envía por UART el reporte de valores normales.
 *
 * @param fc       Frecuencia cardíaca medida.
 * @param temp_x10 Temperatura medida escalada.
 */
void alarma_reporte_normal(uint8_t fc, int16_t temp_x10);

#endif /* ALARMA_H */
