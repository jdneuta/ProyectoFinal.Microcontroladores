/**
 * @file i2c.c
 * @brief Implementación del driver I2C por hardware (MSSP).
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#include "i2c.h"

/** @brief Macro de espera hasta que el módulo MSSP complete la operación actual. */
#define I2C_ESPERAR()   while (SSPSTATbits.BF || (SSPCON2 & 0x1F))

/**
 * @brief Inicializa el módulo MSSP en modo I2C maestro a 100 kHz.
 *
 * - RB0 (SDA) y RB1 (SCL) se configuran como entradas (el MSSP los controla).
 * - SSPCON1: SSPM = 1000 : I2C maestro, reloj = Fosc/(4*(SSPADD+1))
 * - SSPADD = 19 : 100 kHz con Fosc = 8 MHz
 */
void i2c_init(void) {
    TRISBbits.TRISB0 = 1;      // SDA : entrada (controlada por MSSP)
    TRISBbits.TRISB1 = 1;      // SCL : entrada (controlada por MSSP)

    SSPCON1 = 0x28;             // SSPEN=1, SSPM=1000 (I2C maestro)
    SSPCON2 = 0x00;
    SSPADD  = I2C_SSPADD;      // 100 kHz
    SSPSTATbits.SMP = 1;       // Slew rate deshabilitado (100 kHz)
    SSPSTATbits.CKE = 0;       // Niveles I2C estándar
}

/**
 * @brief Genera la condición de START.
 */
void i2c_start(void) {
    I2C_ESPERAR();
    SSPCON2bits.SEN = 1;        // Iniciar START
    I2C_ESPERAR();
}

/**
 * @brief Genera la condición de STOP.
 */
void i2c_stop(void) {
    I2C_ESPERAR();
    SSPCON2bits.PEN = 1;        // Iniciar STOP
    I2C_ESPERAR();
}

/**
 * @brief Genera la condición de REPEATED START.
 */
void i2c_restart(void) {
    I2C_ESPERAR();
    SSPCON2bits.RSEN = 1;       // Repeated start
    I2C_ESPERAR();
}

/**
 * @brief Transmite un byte por I2C y retorna el estado del ACK.
 * @param dato Byte a transmitir.
 * @return 0 si ACK recibido, 1 si NACK.
 */
uint8_t i2c_escribir(uint8_t dato) {
    I2C_ESPERAR();
    SSPBUF = dato;
    I2C_ESPERAR();
    return SSPCON2bits.ACKSTAT; // 0 = ACK, 1 = NACK
}

/**
 * @brief Lee un byte del bus I2C.
 * @param ack true para enviar ACK (seguir leyendo), false para NACK (último byte).
 * @return Byte recibido.
 */
uint8_t i2c_leer(bool ack) {
    uint8_t dato;

    I2C_ESPERAR();
    SSPCON2bits.RCEN = 1;       // Habilitar recepción
    I2C_ESPERAR();

    dato = SSPBUF;

    I2C_ESPERAR();
    SSPCON2bits.ACKDT = ack ? 0 : 1;   // 0 = ACK, 1 = NACK
    SSPCON2bits.ACKEN = 1;              // Enviar ACK/NACK
    I2C_ESPERAR();

    return dato;
}

/**
 * @brief Escribe un valor en un registro de un dispositivo I2C.
 * @param direccion Dirección del dispositivo (7 bits).
 * @param registro  Registro destino.
 * @param valor     Valor a escribir.
 */
void i2c_escribir_registro(uint8_t direccion, uint8_t registro, uint8_t valor) {
    i2c_start();
    i2c_escribir((uint8_t)(direccion << 1));    // Dirección + W
    i2c_escribir(registro);
    i2c_escribir(valor);
    i2c_stop();
}

/**
 * @brief Lee un byte de un registro de un dispositivo I2C.
 * @param direccion Dirección del dispositivo (7 bits).
 * @param registro  Registro a leer.
 * @return Valor del registro.
 */
uint8_t i2c_leer_registro(uint8_t direccion, uint8_t registro) {
    uint8_t valor;

    i2c_start();
    i2c_escribir((uint8_t)(direccion << 1));    // Dirección + W
    i2c_escribir(registro);
    i2c_restart();
    i2c_escribir((uint8_t)((direccion << 1) | 0x01)); // Dirección + R
    valor = i2c_leer(false);                           // Leer + NACK
    i2c_stop();

    return valor;
}

/**
 * @brief Lee múltiples bytes consecutivos de un dispositivo I2C.
 * @param direccion Dirección del dispositivo (7 bits).
 * @param registro  Registro inicial.
 * @param buffer    Buffer de destino.
 * @param cantidad  Número de bytes a leer.
 */
void i2c_leer_burst(uint8_t direccion, uint8_t registro, uint8_t *buffer, uint8_t cantidad) {
    i2c_start();
    i2c_escribir((uint8_t)(direccion << 1));           // Dirección + W
    i2c_escribir(registro);
    i2c_restart();
    i2c_escribir((uint8_t)((direccion << 1) | 0x01)); // Dirección + R

    for (uint8_t i = 0; i < cantidad; i++) {
        // ACK en todos excepto el último
        buffer[i] = i2c_leer(i < (cantidad - 1));
    }

    i2c_stop();
}
