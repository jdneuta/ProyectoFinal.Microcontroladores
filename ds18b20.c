/**
 * @file ds18b20.c
 * @brief Implementación del driver DS18B20 por 1-Wire en RB7.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#include "ds18b20.h"

/* ------------------------------------------------------------------ */
/*  Macros de control del pin 1-Wire                                   */
/* ------------------------------------------------------------------ */

/** @brief Poner la línea en bajo (salida = 0). */
#define OW_LOW()    do { OW_LAT = 0; OW_TRIS = 0; } while(0)

/** @brief Soltar la línea (entrada, la resistencia pullup la lleva a alto). */
#define OW_RELEASE() do { OW_TRIS = 1; } while(0)

/** @brief Leer el estado actual de la línea. */
#define OW_READ()   (OW_PIN)

/* ------------------------------------------------------------------ */
/*  Funciones internas del protocolo 1-Wire                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Genera el pulso de reset y verifica la presencia del sensor.
 * @return true si el sensor respondió con pulso de presencia, false si no.
 */
static bool ow_reset(void) {
    bool presencia;

    OW_LOW();
    __delay_us(480);    // Pulso de reset: mínimo 480 µs

    OW_RELEASE();
    __delay_us(70);     // Esperar pulso de presencia

    presencia = (OW_READ() == 0);   // El sensor jala la línea a bajo

    __delay_us(410);    // Completar el slot de reset (480 µs total desde release)

    return presencia;
}

/**
 * @brief Escribe un bit en el bus 1-Wire.
 * @param bit Valor del bit a escribir (0 o 1).
 */
static void ow_write_bit(uint8_t bit) {
    if (bit) {
        // Escribir '1': pulso bajo de 6 µs, soltar, esperar 64 µs
        OW_LOW();
        __delay_us(6);
        OW_RELEASE();
        __delay_us(64);
    } else {
        // Escribir '0': pulso bajo de 60 µs, soltar, esperar 10 µs
        OW_LOW();
        __delay_us(60);
        OW_RELEASE();
        __delay_us(10);
    }
}

/**
 * @brief Lee un bit del bus 1-Wire.
 * @return Valor del bit leído (0 o 1).
 */
static uint8_t ow_read_bit(void) {
    uint8_t bit;

    OW_LOW();
    __delay_us(6);      // Iniciar slot de lectura
    OW_RELEASE();
    __delay_us(9);      // Esperar que el sensor estabilice el dato

    bit = OW_READ();    // Leer

    __delay_us(55);     // Completar el slot de 70 µs total
    return bit;
}

/**
 * @brief Escribe un byte completo en el bus 1-Wire (LSB primero).
 * @param byte Byte a transmitir.
 */
static void ow_write_byte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        ow_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

/**
 * @brief Lee un byte completo del bus 1-Wire (LSB primero).
 * @return Byte recibido.
 */
static uint8_t ow_read_byte(void) {
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (ow_read_bit()) {
            byte |= (uint8_t)(1 << i);
        }
    }
    return byte;
}

/* ------------------------------------------------------------------ */
/*  Implementaciones públicas                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Inicializa el pin RB7 como entrada (reposo del bus 1-Wire).
 */
void ds18b20_init(void) {
    OW_RELEASE();       // Pin como entrada, pullup externo lleva a VCC
}

/**
 * @brief Lee la temperatura del sensor DS18B20.
 *
 * Secuencia:
 * 1. Reset + verificar presencia
 * 2. Skip ROM (0xCC) : dirigirse al único sensor en el bus
 * 3. Convert T (0x44) : iniciar conversión de temperatura
 * 4. Esperar 750 ms (resolución 12 bits)
 * 5. Reset + Skip ROM + Read Scratchpad (0xBE)
 * 6. Leer 2 bytes del scratchpad y convertir a °C × 10
 *
 * @return Temperatura en décimas de grado (ej. 365 = 36.5 °C),
 *         o DS18B20_ERROR si el sensor no responde.
 */
int16_t ds18b20_leer_temperatura(void) {
    uint8_t byte_low, byte_high;
    int16_t raw;

    // --- Paso 1: reset e iniciar conversión ---
    if (!ow_reset()) return DS18B20_ERROR;

    ow_write_byte(0xCC);    // Skip ROM
    ow_write_byte(0x44);    // Convert T

    // Esperar conversión completa (resolución 12 bits = 750 ms máximo)
    __delay_ms(750);

    // --- Paso 2: leer scratchpad ---
    if (!ow_reset()) return DS18B20_ERROR;

    ow_write_byte(0xCC);    // Skip ROM
    ow_write_byte(0xBE);    // Read Scratchpad

    byte_low  = ow_read_byte();   // Byte 0: LSB de temperatura
    byte_high = ow_read_byte();   // Byte 1: MSB de temperatura

    // No necesitamos los bytes restantes del scratchpad
    ow_reset();             // Reset para liberar el bus

    // --- Paso 3: convertir a °C × 10 ---
    // El registro de temperatura es un entero con signo Q12.4
    // Resolución 12 bits: 1 LSB = 0.0625 °C
    // temp_x10 = raw * 10 / 16 = raw * 5 / 8
    raw = ((int16_t)byte_high << 8) | byte_low;

    return (int16_t)((raw * 5) / 8);
}
