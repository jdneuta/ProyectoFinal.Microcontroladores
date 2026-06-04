#include <xc.h>
#include <stdint.h>
#include "config.h"
#include "sensores.h"
#include "i2c.h"


// Buffer circular de muestras IR para cálculo de BPM
static uint32_t ir_buffer[BUFFER_SIZE];
static uint8_t  buf_index = 0;
static uint8_t  buf_lleno = 0;
 

/**
 * Escribe un byte en un registro del MAX30102.
 * Secuencia: START ? ADDR_W ? REG ? DATA ? STOP
 */
static void I2C_WriteReg(uint8_t reg, uint8_t valor) {
    I2C_Start();
    I2C_Write(MAX30102_ADDR_W);
    I2C_Write(reg);
    I2C_Write(valor);
    I2C_Stop();
}
 
/**
 * Lee un byte de un registro del MAX30102.
 * Secuencia: START ? ADDR_W ? REG ? RESTART ? ADDR_R ? READ ? NACK ? STOP
 * NACK en lectura única: le indica al sensor que no habrá más lecturas.
 */
static uint8_t I2C_ReadReg(uint8_t reg) {
    uint8_t dato;
 
    I2C_Start();
    I2C_Write(MAX30102_ADDR_W);
    I2C_Write(reg);
    I2C_Restart();                  // tu función Repeated START
    I2C_Write(MAX30102_ADDR_R);
    dato = I2C_Read();              // Lee byte (sin ACK interno)
    I2C_Nack();                     // NACK: única lectura, liberar bus
    I2C_Stop();
 
    return dato;
}
 
/**
 * Lee N bytes consecutivos desde un registro base (burst read).
 * Secuencia: START ? ADDR_W ? REG ? RESTART ? ADDR_R
 *            ? [READ+ACK] x (n-1) ? [READ+NACK] ? STOP
 *
 * ACK en cada byte intermedio: le dice al sensor que siga enviando.
 * NACK en el último byte:      le dice al sensor que pare.
 */
static void I2C_ReadBurst(uint8_t reg, uint8_t *buf, uint8_t n) {
    uint8_t i;
 
    I2C_Start();
    I2C_Write(MAX30102_ADDR_W);
    I2C_Write(reg);
    I2C_Restart();                  // tu función Repeated START
    I2C_Write(MAX30102_ADDR_R);
 
    for (i = 0; i < n; i++) {
        buf[i] = I2C_Read();        // Lee byte (sin ACK interno)
        if (i < (n - 1)) {
            I2C_Ack();              // ACK: hay más bytes por leer
        } else {
            I2C_Nack();             // NACK: último byte, liberar bus
        }
    }
 
    I2C_Stop();
}
 
uint8_t MAX30102_Init(void) {
    uint8_t id;
 
    // Verificar que el sensor esté en el bus I2C
    id = MAX30102_GetPartID();
    if (id != MAX30102_PART_ID) {
        return 0;                   // Sensor no encontrado o no responde
    }
 
    // Reset software y esperar estabilización
    MAX30102_Reset();
    __delay_ms(100);
 
    // Limpiar punteros del FIFO
    MAX30102_ClearFIFO();
 
    // Habilitar interrupción FIFO Almost Full (bit 7 de INT_ENABLE1)
    I2C_WriteReg(REG_INT_ENABLE1, 0x80);
    I2C_WriteReg(REG_INT_ENABLE2, 0x00);
 
    // Configurar FIFO: promedio 4 muestras, rollover ON, alerta a 15 libres
    I2C_WriteReg(REG_FIFO_CONFIG, FIFO_CONFIG_VAL);
 
    // Modo SpO2: 2 canales activos (RED + IR)
    I2C_WriteReg(REG_MODE_CONFIG, MODE_SPO2);
 
    // SpO2: ADC range 4096 nA, 100 sps, ancho de pulso 411 µs (18 bits)
    I2C_WriteReg(REG_SPO2_CONFIG, SPO2_CONFIG_VAL);
 
    // Amplitud de los LEDs (~18.6 mA)
    I2C_WriteReg(REG_LED1_PA, LED_AMPLITUDE);   // LED Rojo
    I2C_WriteReg(REG_LED2_PA, LED_AMPLITUDE);   // LED IR
 
    return 1;                       // Inicialización correcta
}
 
void MAX30102_Reset(void) {
    I2C_WriteReg(REG_MODE_CONFIG, MODE_RESET);
    __delay_ms(50);
}
 

// Modo bajo consumo (SHDN=1)
void MAX30102_Shutdown(void) {
    uint8_t reg = I2C_ReadReg(REG_MODE_CONFIG);
    I2C_WriteReg(REG_MODE_CONFIG, reg | MODE_SHDN);
}
 
//  Sale del modo standby (SHDN=0)
void MAX30102_Wakeup(void) {
    uint8_t reg = I2C_ReadReg(REG_MODE_CONFIG);
    I2C_WriteReg(REG_MODE_CONFIG, reg & ~MODE_SHDN);
}
 
//  Reinicia los punteros del FIFO
void MAX30102_ClearFIFO(void) {
    I2C_WriteReg(REG_FIFO_WR_PTR, 0x00);
    I2C_WriteReg(REG_OVF_COUNTER, 0x00);
    I2C_WriteReg(REG_FIFO_RD_PTR, 0x00);
}
 
// ------------------------------------------------------------
//  MAX30102_ReadFIFO
//  Lee una muestra del FIFO: 6 bytes = [RED 3B] + [IR 3B] (18 bits útiles)
//  Guarda IR en buffer circular para cálculo posterior de BPM.
//  Retorna 1 si hay dato nuevo, 0 si el FIFO está vacío.
// ------------------------------------------------------------
uint8_t MAX30102_ReadFIFO(MAX30102_Sample *muestra) {
    uint8_t raw[6];
    uint8_t wr_ptr, rd_ptr;
 
    wr_ptr = I2C_ReadReg(REG_FIFO_WR_PTR);
    rd_ptr = I2C_ReadReg(REG_FIFO_RD_PTR);
 
    // Si los punteros son iguales no hay muestras nuevas
    if (wr_ptr == rd_ptr) {
        return 0;
    }
 
    // Burst read de 6 bytes desde REG_FIFO_DATA
    // ACK en bytes 0-4, NACK en byte 5 (último)
    I2C_ReadBurst(REG_FIFO_DATA, raw, 6);
 
    // Reconstruir valor RED de 18 bits (bits [17:16] en raw[0] & 0x03)
    muestra->red = ((uint32_t)(raw[0] & 0x03) << 16)
                 | ((uint32_t) raw[1]          <<  8)
                 |  (uint32_t) raw[2];
 
    // Reconstruir valor IR de 18 bits
    muestra->ir  = ((uint32_t)(raw[3] & 0x03) << 16)
                 | ((uint32_t) raw[4]          <<  8)
                 |  (uint32_t) raw[5];
 
    // Acumular IR en buffer circular para detección de picos
    ir_buffer[buf_index] = muestra->ir;
    buf_index = (buf_index + 1) % BUFFER_SIZE;
    if (buf_index == 0) buf_lleno = 1;
 
    return 1;
}
 
// ------------------------------------------------------------
//  MAX30102_UsuarioDetectado
//  Criterio: si IR > UMBRAL_IR_DEDO hay dedo sobre el sensor.
//  Hace una lectura interna sin afectar el buffer de BPM.
// ------------------------------------------------------------
uint8_t MAX30102_UsuarioDetectado(void) {
    MAX30102_Sample s;
    if (!MAX30102_ReadFIFO(&s)) return 0;
    return (s.ir > UMBRAL_IR_DEDO) ? 1 : 0;
}
 
// ------------------------------------------------------------
//  MAX30102_CalcularFC
//  Algoritmo de detección de picos sobre el buffer IR circular.
//  Sample rate efectivo = 100 sps / SMP_AVE(4) = 25 muestras/seg.
//  BPM = (25 * 60) / distancia_promedio_entre_picos
// ------------------------------------------------------------
MAX30102_Result MAX30102_CalcularFC(void) {
    MAX30102_Result resultado = {0, 0, 0};
    uint8_t  i;
    uint32_t suma = 0;
    uint32_t promedio;
    uint32_t umbral_pico;
 
    uint8_t  picos    = 0;
    uint8_t  en_pico  = 0;
    uint16_t distancias[MAX_PICOS];
    uint16_t dist_suma = 0;
    uint16_t dist_prom;
    uint16_t bpm;
    uint16_t ultimo_pico = 0;
    uint8_t  n_dist;
 
    // Necesita el buffer completo para un cálculo confiable
    if (!buf_lleno) {
        resultado.valid = 0;
        return resultado;
    }
 
    // Calcular promedio del buffer como línea base
    for (i = 0; i < BUFFER_SIZE; i++) {
        suma += ir_buffer[i];
    }
    promedio = suma / BUFFER_SIZE;
 
    // Umbral = línea base + 60% de la línea base
    umbral_pico = promedio + (promedio / 10UL) * 6UL;
 
    // Detectar flancos ascendentes del umbral (inicio de cada pico)
    for (i = 1; i < BUFFER_SIZE; i++) {
        if (!en_pico && ir_buffer[i] > umbral_pico) {
            en_pico = 1;
            if (picos > 0 && (picos - 1) < MAX_PICOS) {
                distancias[picos - 1] = i - ultimo_pico;
            }
            ultimo_pico = i;
            picos++;
        } else if (en_pico && ir_buffer[i] <= umbral_pico) {
            en_pico = 0;
        }
    }
 
    // Se necesitan al menos 2 picos (1 distancia) para calcular BPM
    if (picos < 2) {
        resultado.valid = 0;
        return resultado;
    }
 
    // Promediar distancias entre picos detectados
    n_dist = picos - 1;
    if (n_dist > MAX_PICOS) n_dist = MAX_PICOS;
 
    for (i = 0; i < n_dist; i++) {
        dist_suma += distancias[i];
    }
    dist_prom = dist_suma / n_dist;
 
    if (dist_prom == 0) {
        resultado.valid = 0;
        return resultado;
    }
 
    // BPM = (muestras_por_segundo × 60) / distancia_entre_picos
    bpm = (25U * 60U) / dist_prom;
 
    // Validar rango fisiológico aceptable
    if (bpm < 30 || bpm > 250) {
        resultado.valid = 0;
        return resultado;
    }
 
    resultado.bpm    = bpm;
    resultado.valid  = 1;
    resultado.alarma = (bpm < FC_MIN || bpm > FC_MAX) ? 1 : 0;
 
    return resultado;
}
/*
// ------------------------------------------------------------
//  MAX30102_LeerTemperaturaChip
//  Temperatura interna del die para diagnóstico (no corporal).
//  Resolución: 0.0625 °C por paso fraccionario.
// ------------------------------------------------------------
float MAX30102_LeerTemperaturaChip(void) {
    uint8_t temp_int;
    uint8_t temp_frac;
 
    // Disparar conversión
    I2C_WriteReg(REG_TEMP_CONFIG, 0x01);
    __delay_ms(30);
 
    temp_int  = I2C_ReadReg(REG_TEMP_INT);
    temp_frac = I2C_ReadReg(REG_TEMP_FRAC);
 
    return (float)temp_int + ((float)(temp_frac & 0x0F) * 0.0625f);
}
 */
// 
uint8_t MAX30102_GetPartID(void) {
    return I2C_ReadReg(REG_PART_ID);
}
