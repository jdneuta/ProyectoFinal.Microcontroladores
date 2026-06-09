/*
 * File:   main.c
 * Author: María De Los Ángeles Castillo
 *
 * Example: PIC18F4550 + MAX30102 + DS18B20 + SSD1306 OLED + UART
 */

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "config.h"
#include "i2c.h"
#include "ssd1306_oled.h"
#include "sensores.h"
#include "uart.h"

void main(void) {
    
    int16_t temp = 0;
    int16_t entero = 0;
    int16_t decimal = 0;

    char line[22];
    char texto_temp[22];
    char uart_text[60];

    uint32_t red = 0;
    uint32_t ir = 0;

    uint8_t contador_temp = 0;
    uint8_t bpm_send = 0;
    uint8_t spo2_send = 0;
    uint8_t alerta = 0;
    uint8_t segundos_sin_reloj = 0;

    MAX30102_Result_t result = {0};

    // Oscilador interno a 8 MHz
    OSCCON = 0x72;

    // Todo digital
    ADCON1 = 0x0F;
    
    // Inicializar LEDs
    LEDs_Init();
    LED_ENCENDIDO_LAT = 1;

    // Pin del DS18B20 como entrada al inicio
    DS18B20_LAT = 0;
    DS18B20_TRIS = 1;
    DS18B20_SetResolution(10);

    // Inicializaciones
    I2C_Init_Master(I2C_100KHZ);
    Uart_Init(9600);
    
    INT2_Init();
    
    OLED_Init();
    OLED_ClearDisplay();
    OLED_SetFont(FONT_1);
    
    LEDs_Apagar_Estados();
    LED_PREPARANDO_LAT = 1;

    OLED_Write_Text_Centered(12, "MONIT SIGNOS VITALES");
    OLED_Write_Text_Centered(24, "MAX + DS18B20 + UART");
    OLED_Write_Text_Centered(36, "Encendiendo...");
    OLED_Update();

    Uart_Send_String("Encendiendo...\r\n");

    __delay_ms(2000);

    if(!MAX30102_Begin()) {
        
        LEDs_Apagar_Estados();
        LED_ALERTA_LAT = 0;
        LED_PREPARANDO_LAT = 1;
        
        OLED_ClearDisplay();
        OLED_Write_Text_Centered(12, "MAX30102 ERROR");
        OLED_Write_Text_Centered(24, "Revisa I2C");
        OLED_Write_Text_Centered(36, "ADDR 0x57");
        OLED_Update();
        __delay_ms(1000);

        while(1){Uart_Send_String("ERROR=MAX30102\r\n"); __delay_ms(3000);}  
    }
    
    LEDs_Apagar_Estados();
    LED_PREPARANDO_LAT = 1;
    
    OLED_ClearDisplay();
    OLED_Write_Text_Centered(28, "Inicializando Sistema");
    OLED_Update();

    Uart_Send_String("Inicializando Sistema\r\n");

    __delay_ms(3000);
    
    temp = DS18B20_ReadTempC_x100();
    
    LEDs_Apagar_Estados();
    LED_FUNCIONAL_LAT = 1;

    while(1) {
        
        // Leer muestras disponibles del MAX30102
        while(MAX30102_GetAvailableSamples() > 0) {
            
            if(MAX30102_ReadFIFO(&red, &ir)) {
                
                MAX30102_ProcessSample(red, ir, &result);
            }
        }

        contador_temp++;

        if(contador_temp >= 3) {
            contador_temp = 0;
            temp = DS18B20_ReadTempC_x100();
        }

        OLED_ClearDisplay();
        
        OLED_Write_Text_Centered(0,  "MONITOR");
        OLED_Write_Text_Centered(12, "SIGNOS VITALES");

        // ==========================
        // Temperatura
        // ==========================
        if(temp == 32767) {
            
            OLED_Write_Text_Centered(52, "Temp: Error");
            sprintf(texto_temp, "Temp:ERR");
        }
        else {
            
            entero = temp / 100;
            decimal = abs(temp % 100);

            sprintf(texto_temp, "Temp:%d.%02d C", entero, decimal);
            
        }

        // ==========================
        // MAX30102 en OLED
        // ==========================
        if(!result.finger_detected) {
            
            OLED_ClearDisplay();
            
            LEDs_Apagar_Estados();
            
            LED_ESPERA_LAT = 1;
            
            OLED_Write_Text_Centered(0,  "MONITOR");
            OLED_Write_Text_Centered(12, "SIGNOS VITALES");
            OLED_Write_Text_Centered(30, "Ponte el reloj");
            
            bpm_send = 0;
            spo2_send = 0;
            
            if(segundos_sin_reloj < TIEMPO_SLEEP_SIN_MANO) {
                segundos_sin_reloj++;
            }

            if(segundos_sin_reloj >= TIEMPO_SLEEP_SIN_MANO) {
                
                Sistema_EntrarSleep();
                LED_ESPERA_LAT = 1;

                segundos_sin_reloj = 0;
                temp = DS18B20_ReadTempC_x100();
            }
        }
        else {
            
            segundos_sin_reloj = 0;
            
            LEDs_Apagar_Estados();
            LED_FUNCIONAL_LAT = 1;
            
            bpm_send = result.bpm_valid ? result.bpm : 0;
            spo2_send = result.spo2_valid ? result.spo2 : 0;

            sprintf(line, "BPM:%u", bpm_send);
            OLED_Write_Text_Centered(28, line);

            sprintf(line, "SpO2:%u%%", spo2_send);
            OLED_Write_Text_Centered(40, line);
            
            OLED_Write_Text_Centered(52, texto_temp);
            
        }
        OLED_Update();
        
        // ==========================
        // Comparar con rangos normales
        // ==========================
        alerta = Verificar_Rangos(temp, bpm_send, spo2_send, result.finger_detected);

        if(alerta) {
            
            LED_ALERTA_LAT = 1;

            OLED_Update();

            OLED_InvertDisplay(1);
            __delay_ms(200);
            OLED_InvertDisplay(0);
            __delay_ms(200);
            OLED_InvertDisplay(1);
            __delay_ms(200);
            OLED_InvertDisplay(0);
        }
        else {
            
            LED_ALERTA_LAT = 0;
            OLED_InvertDisplay(0);
        }
        
        // ==========================
        // Enviar por UART
        // ==========================
        if(temp == 32767) {
            
            sprintf(uart_text,
                    "TEMP:0,BPM:%u,SPO2:%u,ALERTA:%u\r\n",
                    bpm_send,
                    spo2_send,
                    alerta);
        }
        
        else {
            
            sprintf(uart_text,
                    "TEMP:%d.%02d,BPM:%u,SPO2:%u,ALERTA:%u\r\n",
                    entero,
                    decimal,
                    bpm_send,
                    spo2_send,
                    alerta);
        }

        Uart_Send_String(uart_text);

        __delay_ms(1000);
    }
}
