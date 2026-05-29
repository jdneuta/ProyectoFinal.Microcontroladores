/**
 * @file max30102.h
 * @brief Driver para sensor de frecuencia cardíaca MAX30102 por I2C.
 *
 * El MAX30102 usa dirección I2C 0x57. Provee detección de presencia
 * (dedo sobre el sensor) y estimación de frecuencia cardíaca en BPM.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#ifndef MAX30102_H
#define MAX30102_H

#include "config.h"
#include "i2c.h"

/* ------------------------------------------------------------------ */
/*  Dirección y registros del MAX30102                                 */
/* ------------------------------------------------------------------ */

/** @brief Dirección I2C del MAX30102. */
#define MAX30102_ADDR           0x57

/** @brief Registro de interrupción de estado 1. */
#define MAX30102_INT_STATUS1    0x00

/** @brief Registro de configuración FIFO. */
#define MAX30102_FIFO_CONFIG    0x08

/** @brief Registro de configuración de modo. */
#define MAX30102_MODE_CONFIG    0x09

/** @brief Registro de configuración SPO2. */
#define MAX30102_SPO2_CONFIG    0x0A

/** @brief Registro de amplitud LED1 (rojo). */
#define MAX30102_LED1_PA        0x0C

/** @brief Registro de amplitud LED2 (IR). */
#define MAX30102_LED2_PA        0x0D

/** @brief Puntero de escritura del FIFO. */
#define MAX30102_FIFO_WR_PTR    0x04

/** @brief Puntero de lectura del FIFO. */
#define MAX30102_FIFO_RD_PTR    0x06

/** @brief Registro de datos del FIFO. */
#define MAX30102_FIFO_DATA      0x07

/** @brief Registro Part ID (debe leer 0x15 para confirmar dispositivo). */
#define MAX30102_PART_ID        0xFF

/* ------------------------------------------------------------------ */
/*  Umbrales de detección de presencia                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Umbral mínimo de señal IR para considerar que hay un dedo.
 * @note  Valor empírico: ajustar según condiciones reales.
 */
#define MAX30102_UMBRAL_PRESENCIA   50000UL

/* ------------------------------------------------------------------ */
/*  Valor de error                                                     */
/* ------------------------------------------------------------------ */

/** @brief Valor retornado cuando no hay dedo o hay error de lectura. */
#define MAX30102_SIN_DEDO   0xFF

/* ------------------------------------------------------------------ */
/*  Prototipos                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Inicializa el MAX30102 en modo de frecuencia cardíaca.
 *
 * Configura el sensor para operar solo con el LED IR (modo HR),
 * con la corriente y frecuencia de muestreo adecuadas.
 *
 * @return true si el sensor fue detectado y configurado, false si no responde.
 */
bool max30102_init(void);

/**
 * @brief Verifica si hay un dedo apoyado sobre el sensor.
 *
 * Lee el valor IR del FIFO y lo compara con el umbral de presencia.
 *
 * @return true si se detecta presencia, false si no hay dedo.
 */
bool max30102_hay_dedo(void);

/**
 * @brief Lee y retorna la frecuencia cardíaca estimada en BPM.
 *
 * Acumula muestras del FIFO y detecta picos en la señal IR para
 * estimar la frecuencia cardíaca.
 *
 * @return BPM estimado (40 - 200), o MAX30102_SIN_DEDO si no hay señal válida.
 */
uint8_t max30102_leer_fc(void);

#endif /* MAX30102_H */
