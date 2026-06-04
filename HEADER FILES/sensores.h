/* 
 * File:   sensores.h
 * Author: MARIA DE LOS ANGELES
 *
 * Created on 21 de mayo de 2026, 17:46
 */
#ifndef SENSORES_H
#define SENSORES_H

#include <xc.h>
#include <stdint.h>

#define LED_ENCENDIDO       LATAbits.LATA1   // Siempre ON con energía
#define LED_FUNCIONAL       LATAbits.LATA2   // Sistema operando OK
#define LED_PREPARANDO      LATAbits.LATA3   // Solo durante init
#define LED_EN_ESPERA       LATAbits.LATA4   // Sin usuario detectado
#define LED_ALARMA          LATAbits.LATA5   // FC fuera de rango

 
#define BUFFER_SIZE         100              // Muestras IR para detector de picos
#define UMBRAL_IR_DEDO      50000UL          // IR mínimo para considerar dedo presente
#define MAX_PICOS           20               // Máximo de picos a detectar
// ------------------------------------------------------------
//  DIRECCIÓN I2C
// ------------------------------------------------------------
#define MAX30102_ADDR_W     0xAE    // Write (0x57 << 1)
#define MAX30102_ADDR_R     0xAF    // Read

// ------------------------------------------------------------
//  REGISTROS DE ESTADO E INTERRUPCIONES
// ------------------------------------------------------------
#define REG_INT_STATUS1     0x00
#define REG_INT_STATUS2     0x01
#define REG_INT_ENABLE1     0x02
#define REG_INT_ENABLE2     0x03

// ------------------------------------------------------------
//  REGISTROS FIFO
// ------------------------------------------------------------
#define REG_FIFO_WR_PTR     0x04
#define REG_OVF_COUNTER     0x05
#define REG_FIFO_RD_PTR     0x06
#define REG_FIFO_DATA       0x07

// ------------------------------------------------------------
//  REGISTROS DE CONFIGURACIÓN
// ------------------------------------------------------------
#define REG_FIFO_CONFIG     0x08
#define REG_MODE_CONFIG     0x09
#define REG_SPO2_CONFIG     0x0A
#define REG_LED1_PA         0x0C    // Amplitud LED Rojo
#define REG_LED2_PA         0x0D    // Amplitud LED IR
#define REG_PILOT_PA        0x10
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12

// ------------------------------------------------------------
//  REGISTROS DE TEMPERATURA INTERNA
// ------------------------------------------------------------
#define REG_TEMP_INT        0x1F
#define REG_TEMP_FRAC       0x20
#define REG_TEMP_CONFIG     0x21

// ------------------------------------------------------------
//  REGISTRO DE IDENTIFICACIÓN
// ------------------------------------------------------------
#define REG_REV_ID          0xFE
#define REG_PART_ID         0xFF
#define MAX30102_PART_ID    0x15    // Valor esperado

// ------------------------------------------------------------
//  VALORES DE CONFIGURACIÓN
// ------------------------------------------------------------

// REG_MODE_CONFIG (0x09)
#define MODE_HR_ONLY        0x02    // Solo frecuencia cardíaca
#define MODE_SPO2           0x03    // SpO2 + FC
#define MODE_MULTI_LED      0x07
#define MODE_RESET          0x40    // Reset software
#define MODE_SHDN           0x80    // Apagado (standby)

// REG_SPO2_CONFIG (0x0A)
// ADC Range: 00=2048, 01=4096, 10=8192, 11=16384 nA
// Sample Rate: 000=50, 001=100, 010=200, 011=400 sps
// Pulse Width: 00=69µs(15bit), 01=118µs(16bit),
//              10=215µs(17bit), 11=411µs(18bit)
#define SPO2_ADC_RGE_4096   0x20
#define SPO2_SR_100         0x04
#define SPO2_PW_411         0x03
#define SPO2_CONFIG_VAL    (SPO2_ADC_RGE_4096 | SPO2_SR_100 | SPO2_PW_411)

// REG_FIFO_CONFIG (0x08)
// SMP_AVE: 000=1, 001=2, 010=4, 011=8, 100=16, 101=32
#define FIFO_SMP_AVE_4      0x40    // Promedio de 4 muestras
#define FIFO_ROLLOVER_EN    0x10    // FIFO circular
#define FIFO_A_FULL_F       0x0F    // Alerta casi lleno
#define FIFO_CONFIG_VAL    (FIFO_SMP_AVE_4 | FIFO_ROLLOVER_EN | FIFO_A_FULL_F)

// Amplitud LEDs: 0x00=0mA ... 0xFF=51mA  (paso ~0.2mA)
#define LED_AMPLITUDE       0x5F    // ~18.6 mA

// ------------------------------------------------------------
//  UMBRALES FRECUENCIA CARDÍACA
// ------------------------------------------------------------
#define FC_MIN              60      // bpm mínimo normal
#define FC_MAX              100     // bpm máximo normal

// ------------------------------------------------------------
//  ESTRUCTURA DE DATOS
// ------------------------------------------------------------
typedef struct {
    uint32_t red;           // Canal LED Rojo  (18 bits)
    uint32_t ir;            // Canal LED IR    (18 bits)
} MAX30102_Sample;

typedef struct {
    uint16_t bpm;           // Frecuencia cardíaca calculada
    uint8_t  valid;         // 1 = lectura válida, 0 = inválida
    uint8_t  alarma;        // 1 = fuera de rango normal
} MAX30102_Result;


uint8_t  MAX30102_Init(void);
void     MAX30102_Reset(void);
void     MAX30102_Shutdown(void);
void     MAX30102_Wakeup(void);
uint8_t  MAX30102_ReadFIFO(MAX30102_Sample *muestra);
void     MAX30102_ClearFIFO(void);
uint8_t  MAX30102_UsuarioDetectado(void);
MAX30102_Result MAX30102_CalcularFC(void);
float    MAX30102_LeerTemperaturaChip(void);
uint8_t  MAX30102_GetPartID(void);

#endif 