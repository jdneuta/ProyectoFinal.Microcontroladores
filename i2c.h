/**
 * @file i2c.h
 * @brief Driver I2C por hardware (módulo MSSP) para PIC18F4550.
 *
 * Usa RB0 como SDA y RB1 como SCL. Provee las operaciones básicas
 * del bus I2C: start, stop, escritura y lectura de bytes, así como
 * funciones de alto nivel para leer y escribir registros de dispositivos.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#ifndef I2C_H
#define I2C_H

#include "config.h"

/* ------------------------------------------------------------------ */
/*  Configuración de velocidad                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Valor de SSPADD para 100 kHz con Fosc = 8 MHz.
 * @note  Fórmula: SSPADD = (Fosc / (4 * frecuencia_I2C)) - 1 = 19
 */
#define I2C_SSPADD  19

/* ------------------------------------------------------------------ */
/*  Prototipos                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Inicializa el módulo MSSP en modo I2C maestro a 100 kHz.
 */
void i2c_init(void);

/**
 * @brief Genera la condición de START en el bus I2C.
 */
void i2c_start(void);

/**
 * @brief Genera la condición de STOP en el bus I2C.
 */
void i2c_stop(void);

/**
 * @brief Genera la condición de REPEATED START en el bus I2C.
 */
void i2c_restart(void);

/**
 * @brief Transmite un byte por el bus I2C.
 * @param dato Byte a transmitir.
 * @return 0 si se recibió ACK, 1 si se recibió NACK.
 */
uint8_t i2c_escribir(uint8_t dato);

/**
 * @brief Lee un byte del bus I2C.
 * @param ack true para enviar ACK (hay más bytes), false para NACK (último byte).
 * @return Byte recibido.
 */
uint8_t i2c_leer(bool ack);

/**
 * @brief Escribe un valor en un registro de un dispositivo I2C.
 * @param direccion Dirección I2C del dispositivo (7 bits).
 * @param registro  Registro de destino.
 * @param valor     Valor a escribir.
 */
void i2c_escribir_registro(uint8_t direccion, uint8_t registro, uint8_t valor);

/**
 * @brief Lee un byte de un registro de un dispositivo I2C.
 * @param direccion Dirección I2C del dispositivo (7 bits).
 * @param registro  Registro a leer.
 * @return Valor leído del registro.
 */
uint8_t i2c_leer_registro(uint8_t direccion, uint8_t registro);

/**
 * @brief Lee múltiples bytes consecutivos de un dispositivo I2C.
 * @param direccion Dirección I2C del dispositivo (7 bits).
 * @param registro  Registro inicial.
 * @param buffer    Puntero al buffer donde se almacenan los datos.
 * @param cantidad  Número de bytes a leer.
 */
void i2c_leer_burst(uint8_t direccion, uint8_t registro, uint8_t *buffer, uint8_t cantidad);

#endif /* I2C_H */
