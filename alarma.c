/**
 * @file alarma.c
 * @brief Implementación del módulo de alarma.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#include "alarma.h"

/**
 * @brief Evalúa si FC o temperatura están fuera de los umbrales normales.
 *
 * @param fc       Frecuencia cardíaca en BPM.
 * @param temp_x10 Temperatura en °C × 10.
 * @return true si algún valor está fuera de rango.
 */
bool alarma_evaluar(uint8_t fc, int16_t temp_x10) {
    bool fc_fuera   = (fc < FC_MIN || fc > FC_MAX);
    bool temp_fuera = (temp_x10 < TEMP_MIN || temp_x10 > TEMP_MAX);
    return (fc_fuera || temp_fuera);
}

/**
 * @brief Activa el LED de alarma con parpadeo y envía aviso por UART.
 *
 * Informa específicamente qué variable está fuera de rango.
 *
 * @param fc       Frecuencia cardíaca medida.
 * @param temp_x10 Temperatura medida (°C × 10).
 */
void alarma_activar(uint8_t fc, int16_t temp_x10) {
    bool fc_fuera   = (fc < FC_MIN || fc > FC_MAX);
    bool temp_fuera = (temp_x10 < TEMP_MIN || temp_x10 > TEMP_MAX);

    // Parpadeo del LED de alarma
    leds_parpadeo_alarma(6);
    LED_ALARMA = 1;     // Dejar encendido después del parpadeo

    // Reporte UART
    uart_escribir_cadena("=== ALARMA ===\r\n");

    if (fc_fuera) {
        uart_escribir_cadena("FC fuera de rango: ");
        uart_escribir_uint(fc);
        uart_escribir_cadena(" BPM\r\n");
    }

    if (temp_fuera) {
        uart_escribir_cadena("Temp fuera de rango: ");
        uart_escribir_temperatura(temp_x10);
    }

    uart_escribir_cadena("==============\r\n");
}

/**
 * @brief Reporta valores normales por UART y apaga el LED de alarma.
 *
 * @param fc       Frecuencia cardíaca medida.
 * @param temp_x10 Temperatura medida (°C × 10).
 */
void alarma_reporte_normal(uint8_t fc, int16_t temp_x10) {
    LED_ALARMA = 0;

    uart_escribir_cadena("FC: ");
    uart_escribir_uint(fc);
    uart_escribir_cadena(" BPM | Temp: ");
    uart_escribir_temperatura(temp_x10);
}
