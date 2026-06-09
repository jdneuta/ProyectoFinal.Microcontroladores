/* 
 * File:   sensores.h
 * Author: María De Los Ángeles Castillo 
 *
 * Created on 29 de mayo de 2026, 09:51 AM
 */

#ifndef SENSORES_H
#define	SENSORES_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"


#define TIEMPO_SLEEP_SIN_MANO 15
// ==========================
// Valores de histéresis
// ==========================
#define TEMP_MIN            34    // 34°C
#define TEMP_MAX            38    // 38°C

#define BPM_MIN             60
#define BPM_MAX             150

#define SPO2_MIN            80


/* The user's I2C library sends 8-bit I2C addresses directly. */
#define MAX30102_ADDR_WR        0xAE
#define MAX30102_ADDR_RD        0xAF
#define MAX30102_EXPECTED_ID    0x15

/* Registers */
#define MAX30102_REG_INTR_STATUS_1     0x00
#define MAX30102_REG_INTR_STATUS_2     0x01
#define MAX30102_REG_INTR_ENABLE_1     0x02
#define MAX30102_REG_INTR_ENABLE_2     0x03
#define MAX30102_REG_FIFO_WR_PTR       0x04
#define MAX30102_REG_OVF_COUNTER       0x05
#define MAX30102_REG_FIFO_RD_PTR       0x06
#define MAX30102_REG_FIFO_DATA         0x07
#define MAX30102_REG_FIFO_CONFIG       0x08
#define MAX30102_REG_MODE_CONFIG       0x09
#define MAX30102_REG_SPO2_CONFIG       0x0A
#define MAX30102_REG_LED1_PA           0x0C   /* Red */
#define MAX30102_REG_LED2_PA           0x0D   /* IR */
#define MAX30102_REG_MULTI_LED_1       0x11
#define MAX30102_REG_MULTI_LED_2       0x12
#define MAX30102_REG_TEMP_INT          0x1F
#define MAX30102_REG_TEMP_FRAC         0x20
#define MAX30102_REG_TEMP_CONFIG       0x21
#define MAX30102_REG_REV_ID            0xFE
#define MAX30102_REG_PART_ID           0xFF

/* Default acquisition settings */
#define MAX30102_SAMPLE_RATE_HZ        100u
#define MAX30102_FINGER_THRESHOLD      50000UL

//============================DS18B20===========================================
#define DS18B20_TRIS     TRISBbits.TRISB7
#define DS18B20_LAT      LATBbits.LATB7
#define DS18B20_PORT     PORTBbits.RB7

//============================LEDS DE ESTADO====================================
#define LED_ENCENDIDO_LAT      LATAbits.LATA5
#define LED_PREPARANDO_LAT     LATAbits.LATA4
#define LED_FUNCIONAL_LAT      LATAbits.LATA3
#define LED_ESPERA_LAT         LATAbits.LATA2

#define LED_ENCENDIDO_TRIS     TRISAbits.TRISA5
#define LED_PREPARANDO_TRIS    TRISAbits.TRISA4
#define LED_FUNCIONAL_TRIS     TRISAbits.TRISA3
#define LED_ESPERA_TRIS        TRISAbits.TRISA2

// ========================== LED de alerta ====================================
#define LED_ALERTA_LAT          LATAbits.LATA1
#define LED_ALERTA_TRIS         TRISAbits.TRISA1

typedef struct
{
    uint32_t red_raw;
    uint32_t ir_raw;
    uint8_t bpm;
    uint8_t spo2;
    bool finger_detected;
    bool bpm_valid;
    bool spo2_valid;
} MAX30102_Result_t;

bool MAX30102_WriteReg(uint8_t reg, uint8_t value);
uint8_t MAX30102_ReadReg(uint8_t reg);
bool MAX30102_ReadBytes(uint8_t reg, uint8_t *buffer, uint8_t length);

bool MAX30102_Begin(void);
void MAX30102_Reset(void);
void MAX30102_Shutdown(bool enable);
void MAX30102_ClearFIFO(void);
uint8_t MAX30102_GetPartID(void);
uint8_t MAX30102_GetAvailableSamples(void);
bool MAX30102_ReadFIFO(uint32_t *red, uint32_t *ir);

void MAX30102_SetupSpO2(void);
void MAX30102_SetLEDPulseAmplitude(uint8_t red_pa, uint8_t ir_pa);

void MAX30102_AlgorithmReset(void);
void MAX30102_ProcessSample(uint32_t red, uint32_t ir, MAX30102_Result_t *result);


bool DS18B20_Start(void);
void DS18B20_WriteByte(uint8_t data);
uint8_t DS18B20_ReadByte(void);
int16_t DS18B20_ReadRaw(void);
int16_t DS18B20_ReadTempC_x100(void);
void DS18B20_SetResolution(uint8_t resolution);


void LEDs_Init(void);
void LEDs_Apagar_Estados(void);

uint8_t Verificar_Rangos(int16_t temp_x100, uint8_t bpm, uint8_t spo2, uint8_t mano_detectada);

void INT2_Init(void);
void Sistema_EntrarSleep(void);

#endif	/* SENSORES_H */

