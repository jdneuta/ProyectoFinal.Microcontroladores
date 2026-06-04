/**
 * @file main.c
 * @brief Monitor portátil de signos vitales basado en PIC18F4550.
 *
 * Ciclo principal del sistema:
 * 1. Inicialización de periféricos y LEDs.
 * 2. Espera detección de dedo sobre el MAX30102.
 * 3. Lectura de FC (MAX30102) y temperatura (DS18B20).
 * 4. Evaluación de umbrales y activación de alarma si corresponde.
 * 5. Visualización en OLED y reporte por UART.
 * 6. Repetir.
 *
 * @author  Juan Diego Neuta
 * @date    2026
 */

#include "config.h"
#include "leds.h"
#include "uart.h"
#include "i2c.h"
#include "oled.h"
#include "max30102.h"
#include "ds18b20.h"
#include "alarma.h"

/* ------------------------------------------------------------------ */
/*  Función principal                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Punto de entrada del programa.
 *
 * Configura el oscilador interno a 8 MHz, inicializa todos los módulos
 * y ejecuta el ciclo de medición indefinidamente.
 */
void main(void) {
    /* ---- Configurar oscilador interno a 8 MHz ---- */
    OSCCONbits.IRCF = 0b111;    // 8 MHz
    OSCCONbits.SCS  = 0b10;     // Fuente: oscilador interno

    /* ---- Inicializar módulos ---- */
    leds_init();
    LED_ENCENDIDO = 1;          // El sistema tiene alimentación

    uart_init();
    uart_escribir_cadena("\r\nMonitor de Signos Vitales\r\n");
    uart_escribir_cadena("Inicializando...\r\n");

    LED_PREPARANDO = 1;         // Inicio de configuración

    i2c_init();
    ds18b20_init();
    oled_init();

    /* ---- Inicializar MAX30102, reintentar si no responde ---- */
    while (!max30102_init()) {
        uart_escribir_cadena("MAX30102 no detectado. Reintentando...\r\n");
        __delay_ms(1000);
    }

    LED_PREPARANDO = 0;         // Configuración completada
    uart_escribir_cadena("Sistema listo.\r\n");

    /* ---- Ciclo principal ---- */
    while (1) {

        /* -- Esperar presencia de dedo (standby automático) -- */
        if (!max30102_hay_dedo()) {
            LED_FUNCIONAL = 0;
            LED_EN_ESPERA = 1;
            oled_escribir_linea(0, "Esperando dedo..");
            oled_escribir_linea(2, "                ");
            oled_escribir_linea(4, "                ");
            oled_escribir_linea(6, "                ");
            __delay_ms(STANDBY_MS);
            continue;
        }

        /* -- Hay dedo: pasar a modo funcional -- */
        LED_EN_ESPERA = 0;
        LED_FUNCIONAL = 1;

        /* -- Leer sensores -- */
        uint8_t fc       = max30102_leer_fc();
        int16_t temp_x10 = ds18b20_leer_temperatura();

        /* -- Validar lecturas -- */
        if (fc == MAX30102_SIN_DEDO || temp_x10 == DS18B20_ERROR) {
            uart_escribir_cadena("Error de lectura. Reintentando...\r\n");
            __delay_ms(PERIODO_MS);
            continue;
        }

        /* -- Evaluar umbrales -- */
        bool hay_alarma = alarma_evaluar(fc, temp_x10);

        if (hay_alarma) {
            alarma_activar(fc, temp_x10);
        } else {
            alarma_reporte_normal(fc, temp_x10);
        }

        /* -- Mostrar en OLED -- */
        oled_mostrar_vitales(fc, temp_x10, hay_alarma);

        /* -- Esperar siguiente ciclo -- */
        __delay_ms(PERIODO_MS);
    }
}
