/**
 * @file max30102.c
 * @brief ImplementaciÛn del driver MAX30102.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#include "max30102.h"

/* ------------------------------------------------------------------ */
/*  Buffer circular de muestras IR para detecciÛn de picos            */
/* ------------------------------------------------------------------ */

/** @brief TamaÒo del buffer de muestras IR. */
#define BUFFER_SIZE     32

/** @brief Buffer de valores IR crudos. */
static uint32_t buffer_ir[BUFFER_SIZE];

/** @brief √çndice actual del buffer. */
static uint8_t buffer_idx = 0;

/* ------------------------------------------------------------------ */
/*  Funciones internas                                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Lee una muestra cruda de 18 bits del FIFO del MAX30102.
 * @return Valor IR de 18 bits leÌdo del FIFO.
 */
static uint32_t max30102_leer_raw(void) {
    uint8_t datos[3];
    i2c_leer_burst(MAX30102_ADDR, MAX30102_FIFO_DATA, datos, 3);
    return ((uint32_t)(datos[0] & 0x03) << 16)
         | ((uint32_t)datos[1] << 8)
         |  (uint32_t)datos[2];
}

/**
 * @brief Calcula el valor mÌnimo del buffer IR.
 * @return Valor mÌnimo encontrado.
 */
static uint32_t buffer_minimo(void) {
    uint32_t min = buffer_ir[0];
    for (uint8_t i = 1; i < BUFFER_SIZE; i++) {
        if (buffer_ir[i] < min) min = buffer_ir[i];
    }
    return min;
}

/**
 * @brief Calcula el valor m·ximo del buffer IR.
 * @return Valor m·ximo encontrado.
 */
static uint32_t buffer_maximo(void) {
    uint32_t max = buffer_ir[0];
    for (uint8_t i = 1; i < BUFFER_SIZE; i++) {
        if (buffer_ir[i] > max) max = buffer_ir[i];
    }
    return max;
}

/* ------------------------------------------------------------------ */
/*  Implementaciones p˙blicas                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Inicializa el MAX30102 en modo frecuencia cardÌaca.
 *
 * Verifica el Part ID (0x15), resetea el sensor, y lo configura:
 * - Modo: HR (solo LED IR)
 * - Corriente LED: ~7 mA (0x24)
 * - Frecuencia de muestreo: 100 Hz
 * - ResoluciÛn ADC: 18 bits
 *
 * @return true si la inicializaciÛn fue exitosa.
 */
bool max30102_init(void) {
    // Verificar Part ID
    uint8_t id = i2c_leer_registro(MAX30102_ADDR, MAX30102_PART_ID);
    if (id != 0x15) return false;

    // Reset del sensor
    i2c_escribir_registro(MAX30102_ADDR, MAX30102_MODE_CONFIG, 0x40);
    __delay_ms(50);

    // Configurar FIFO: sin promediado, FIFO rollover habilitado
    i2c_escribir_registro(MAX30102_ADDR, MAX30102_FIFO_CONFIG, 0x4F);

    // Modo HR (solo LED IR = LED2)
    i2c_escribir_registro(MAX30102_ADDR, MAX30102_MODE_CONFIG, 0x02);

    // SPO2: ADC 18 bits, 400 Hz de muestreo, ancho de pulso 411 µs
    i2c_escribir_registro(MAX30102_ADDR, MAX30102_SPO2_CONFIG, 0x27);

    // Corriente LED IR ~7 mA
    i2c_escribir_registro(MAX30102_ADDR, MAX30102_LED2_PA, 0x24);

    // Limpiar punteros FIFO
    i2c_escribir_registro(MAX30102_ADDR, MAX30102_FIFO_WR_PTR, 0x00);
    i2c_escribir_registro(MAX30102_ADDR, MAX30102_FIFO_RD_PTR, 0x00);

    return true;
}

/**
 * @brief Verifica presencia de dedo comparando seÒal IR con umbral.
 *
 * @return true si la seÒal IR supera MAX30102_UMBRAL_PRESENCIA.
 */
bool max30102_hay_dedo(void) {
    uint32_t muestra = max30102_leer_raw();
    return (muestra > MAX30102_UMBRAL_PRESENCIA);
}

/**
 * @brief Estima la frecuencia cardÌaca en BPM mediante detecciÛn de picos.
 *
 * Acumula BUFFER_SIZE muestras a ~100 ms entre cada una, luego cuenta
 * los cruces por la mitad del rango (min+max)/2 para estimar los picos.
 * BPM = (picos_detectados / 2) * (60000 / tiempo_total_ms)
 *
 * @return BPM en rango 40 - 200, o MAX30102_SIN_DEDO si la seÒal es inv·lida.
 */
uint8_t max30102_leer_fc(void) {
    // Acumular muestras
    for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
        buffer_ir[i] = max30102_leer_raw();
        __delay_ms(10);     // ~100 Hz de muestreo
    }

    uint32_t minimo = buffer_minimo();
    uint32_t maximo = buffer_maximo();

    // Si la amplitud es muy baja, no hay dedo v·lido
    if ((maximo - minimo) < 500) return MAX30102_SIN_DEDO;

    uint32_t umbral = minimo + (maximo - minimo) / 2;

    // Contar cruces ascendentes por el umbral (= n˙mero de picos)
    uint8_t picos = 0;
    bool encima = false;

    for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
        if (!encima && buffer_ir[i] > umbral) {
            picos++;
            encima = true;
        } else if (buffer_ir[i] < umbral) {
            encima = false;
        }
    }

    // Tiempo total de muestreo: BUFFER_SIZE * 10 ms = 320 ms
    // BPM = picos * (60000 / 320) = picos * 187
    // Simplificado para evitar overflow en uint8_t:
    uint16_t bpm = (uint16_t)picos * 187u / 10u;

    if (bpm < 40 || bpm > 200) return MAX30102_SIN_DEDO;

    return (uint8_t)bpm;
}
