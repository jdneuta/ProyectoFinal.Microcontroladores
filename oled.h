/**
 * @file oled.h
 * @brief Driver para pantalla OLED SSD1306 128 x 64 por I2C.
 *
 * Provee funciones para inicializar la pantalla, limpiarla y mostrar
 * cadenas de texto en posiciones de fila definidas.
 * Dirección I2C del SSD1306: 0x3C.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#ifndef OLED_H
#define OLED_H

#include "config.h"
#include "i2c.h"

/* ------------------------------------------------------------------ */
/*  Constantes del SSD1306                                             */
/* ------------------------------------------------------------------ */

/** @brief Dirección I2C del display OLED SSD1306. */
#define OLED_DIRECCION      0x3C

/** @brief Byte de control: el siguiente byte es un comando. */
#define OLED_CMD            0x00

/** @brief Byte de control: el siguiente byte es dato (pixel). */
#define OLED_DATO           0x40

/** @brief Ancho del display en píxeles. */
#define OLED_ANCHO          128

/** @brief Número de páginas (filas de 8 píxeles). */
#define OLED_PAGINAS        8

/* ------------------------------------------------------------------ */
/*  Prototipos                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Inicializa el display OLED SSD1306.
 *
 * Envía la secuencia de comandos de configuración estándar para
 * activar el display en modo normal (negro sobre fondo negro : texto blanco).
 */
void oled_init(void);

/**
 * @brief Limpia toda la pantalla (pone todos los píxeles en negro).
 */
void oled_limpiar(void);

/**
 * @brief Posiciona el cursor en una fila y columna específicas.
 * @param pagina  Fila (0 - 7), cada una tiene 8 píxeles de alto.
 * @param columna Columna inicial (0 - 127).
 */
void oled_set_cursor(uint8_t pagina, uint8_t columna);

/**
 * @brief Escribe un carácter ASCII en la posición actual del cursor.
 * @param c Carácter a mostrar (32 - 126).
 */
void oled_escribir_char(char c);

/**
 * @brief Escribe una cadena de texto en una fila específica.
 * @param pagina  Fila del display (0 - 7).
 * @param cadena  Cadena de texto terminada en '\0'.
 */
void oled_escribir_linea(uint8_t pagina, const char *cadena);

/**
 * @brief Muestra los datos de signos vitales en el display.
 * @param fc       Frecuencia cardíaca en BPM.
 * @param temp_x10 Temperatura escalada (°C × 10).
 * @param alarma   true si hay condición de alarma activa.
 */
void oled_mostrar_vitales(uint8_t fc, int16_t temp_x10, bool alarma);

#endif /* OLED_H */
