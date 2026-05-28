/**
 * @file uart.h
 * @brief Driver EUSART para comunicación serial por RC6 (TX) y RC7 (RX).
 *
 * Provee funciones de inicialización y transmisión de datos hacia una
 * terminal serial externa a 9600 baudios con oscilador de 8 MHz.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#ifndef UART_H
#define UART_H

#include "config.h"

/* ------------------------------------------------------------------ */
/*  Configuración de baudrate                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Valor del registro SPBRG para 9600 baud a 8 MHz.
 * @note  Fórmula (BRGH=1): SPBRG = (Fosc / (16 * baud)) - 1 = 51
 */
#define UART_SPBRG  51

/* ------------------------------------------------------------------ */
/*  Prototipos                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Inicializa el módulo EUSART a 9600 baudios, 8N1.
 *
 * Configura TX (RC6) como salida y RX (RC7) como entrada.
 * Habilita solo transmisión (recepción no se usa en este proyecto).
 */
void uart_init(void);

/**
 * @brief Transmite un único byte por UART.
 * @param dato Byte a transmitir.
 */
void uart_escribir_byte(uint8_t dato);

/**
 * @brief Transmite una cadena de caracteres terminada en '\0'.
 * @param cadena Puntero a la cadena a transmitir.
 */
void uart_escribir_cadena(const char *cadena);

/**
 * @brief Transmite un entero sin signo de 16 bits como texto decimal.
 * @param valor Valor a transmitir.
 */
void uart_escribir_uint(uint16_t valor);

/**
 * @brief Transmite una temperatura escalada (°C × 10) como "XX.X °C".
 * @param temp_x10 Temperatura en décimas de grado (ej. 365 : "36.5 C").
 */
void uart_escribir_temperatura(int16_t temp_x10);

#endif /* UART_H */
