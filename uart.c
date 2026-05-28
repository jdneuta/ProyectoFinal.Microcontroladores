/**
 * @file uart.c
 * @brief Implementación del driver EUSART.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#include "uart.h"

/**
 * @brief Inicializa el EUSART del PIC18F4550 a 9600 baudios, 8N1.
 *
 * - RC6 : TX (salida automática al habilitar TXEN)
 * - RC7 : RX (entrada, no se usa pero se configura correctamente)
 * - BRGH = 1 (alta velocidad) para mejor precisión a 8 MHz
 * - BRG16 = 0 (registro de 8 bits)
 */
void uart_init(void) {
    TRISCbits.TRISC6 = 0;   // TX : salida
    TRISCbits.TRISC7 = 1;   // RX : entrada

    SPBRG  = UART_SPBRG;    // Baudrate 9600

    TXSTAbits.BRGH  = 1;    // Alta velocidad
    TXSTAbits.SYNC  = 0;    // Modo asíncrono
    TXSTAbits.TXEN  = 1;    // Habilitar transmisor

    RCSTAbits.SPEN  = 1;    // Habilitar puerto serial
}

/**
 * @brief Espera a que el buffer de transmisión esté libre y envía un byte.
 * @param dato Byte a transmitir.
 */
void uart_escribir_byte(uint8_t dato) {
    while (!TXSTAbits.TRMT);    // Esperar buffer vacío
    TXREG = dato;
}

/**
 * @brief Transmite una cadena de caracteres terminada en '\0'.
 * @param cadena Puntero a la cadena a transmitir.
 */
void uart_escribir_cadena(const char *cadena) {
    while (*cadena != '\0') {
        uart_escribir_byte((uint8_t)(*cadena));
        cadena++;
    }
}

/**
 * @brief Transmite un entero sin signo de 16 bits como texto decimal.
 *
 * Convierte el valor a dígitos ASCII sin usar sprintf para evitar
 * dependencia de la librería estándar completa en el PIC.
 *
 * @param valor Valor a transmitir (0 - 65535).
 */
void uart_escribir_uint(uint16_t valor) {
    char buffer[6];             // Máximo 5 dígitos + '\0'
    uint8_t i = 0;

    if (valor == 0) {
        uart_escribir_byte('0');
        return;
    }

    // Construir dígitos en orden inverso
    while (valor > 0) {
        buffer[i++] = '0' + (valor % 10);
        valor /= 10;
    }

    // Transmitir en orden correcto
    while (i > 0) {
        uart_escribir_byte((uint8_t)buffer[--i]);
    }
}

/**
 * @brief Transmite una temperatura escalada como "XX.X C\r\n".
 *
 * @param temp_x10 Temperatura en décimas de grado (ej. 365 : "36.5 C").
 */
void uart_escribir_temperatura(int16_t temp_x10) {
    if (temp_x10 < 0) {
        uart_escribir_byte('-');
        temp_x10 = -temp_x10;
    }
    uart_escribir_uint((uint16_t)(temp_x10 / 10));
    uart_escribir_byte('.');
    uart_escribir_byte('0' + (uint8_t)(temp_x10 % 10));
    uart_escribir_cadena(" C\r\n");
}
