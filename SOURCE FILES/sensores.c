#include "config.h"
#include "sensores.h"
#include "uart.h"
#include "ssd1306_oled.h"


//=============================MAX30102=========================================
/* Internal algorithm state */
static uint32_t sample_index = 0;

static int32_t ir_dc = 0;
static int32_t red_dc = 0;
static int32_t ir_prev2 = 0;
static int32_t ir_prev1 = 0;
static uint32_t last_peak_sample = 0;
static uint8_t bpm_value = 0;
static bool bpm_valid_state = false;

static uint16_t spo2_count = 0;
static uint32_t ir_min = 0xFFFFFFFFUL;
static uint32_t ir_max = 0;
static uint32_t red_min = 0xFFFFFFFFUL;
static uint32_t red_max = 0;
static uint8_t spo2_value = 0;
static bool spo2_valid_state = false;

bool MAX30102_WriteReg(uint8_t reg, uint8_t value)
{
    bool ok = true;                                 //Supone que la comunicacion es correcta

    I2C_Start();                                   
    
    //VERIFICA QUE EL MAX RECIBIO CORRECTAMENTE LOS DATOS
    if(I2C_Write(MAX30102_ADDR_WR)) ok = false;    
    if(I2C_Write(reg)) ok = false;
    if(I2C_Write(value)) ok = false;
    I2C_Stop();

    return ok;
}

uint8_t MAX30102_ReadReg(uint8_t reg)       
{
    uint8_t value;                          // Almacena Bit leido  

    I2C_Start();
    I2C_Write(MAX30102_ADDR_WR);            // Envia direccion de escribir de MAX
    I2C_Write(reg);                         // " Direccion donde quiero leer 
    I2C_Restart();                          // Cambia modo sin perder bus 
    I2C_Write(MAX30102_ADDR_RD);            // Envia direccion de leer de MAX
    value = I2C_Read();                     // Sensor coloca el byte en SDA y el PIC lo recibe
    I2C_Nack();                             // Como solo estamos leyendo un único byte, enviamos NACK
    I2C_Stop();

    return value;
}

bool MAX30102_ReadBytes(uint8_t reg, uint8_t *buffer, uint8_t length)
{
    uint8_t i;
    bool ok = true;

    if(length == 0) return false;                   // Si es de 0 la función termina

    I2C_Start();
    
    // VERIFICA QUE EL MAX RECIBIO CORRECTAMENTE 
    if(I2C_Write(MAX30102_ADDR_WR)) ok = false;    
    if(I2C_Write(reg)) ok = false;
    I2C_Restart();                                  
    if(I2C_Write(MAX30102_ADDR_RD)) ok = false;

    for(i = 0; i < length; i++)
    {
        buffer[i] = I2C_Read();

        if(i == (length - 1))
        {
            I2C_Nack();
        }
        else
        {
            I2C_Ack();
        }
    }

    I2C_Stop();
    return ok;
}

uint8_t MAX30102_GetPartID(void)            // verifica sensor conectado 
{
    return MAX30102_ReadReg(MAX30102_REG_PART_ID);
}

void MAX30102_Reset(void)
{
    MAX30102_WriteReg(MAX30102_REG_MODE_CONFIG, 0x40);
    __delay_ms(100);
}

void MAX30102_Shutdown(bool enable)             // Bajo consumo 
{
    uint8_t mode = MAX30102_ReadReg(MAX30102_REG_MODE_CONFIG);

    if(enable)
    {
        mode |= 0x80;
    }
    else
    {
        mode &= (uint8_t)~0x80;
    }

    MAX30102_WriteReg(MAX30102_REG_MODE_CONFIG, mode);
}

void MAX30102_ClearFIFO(void)
{
    MAX30102_WriteReg(MAX30102_REG_FIFO_WR_PTR, 0x00);
    MAX30102_WriteReg(MAX30102_REG_OVF_COUNTER, 0x00);
    MAX30102_WriteReg(MAX30102_REG_FIFO_RD_PTR, 0x00);
}

void MAX30102_SetLEDPulseAmplitude(uint8_t red_pa, uint8_t ir_pa)
{
    MAX30102_WriteReg(MAX30102_REG_LED1_PA, red_pa);
    MAX30102_WriteReg(MAX30102_REG_LED2_PA, ir_pa);
}

void MAX30102_SetupSpO2(void)
{
    /* Clear pending interrupts by reading status registers. */
    (void)MAX30102_ReadReg(MAX30102_REG_INTR_STATUS_1);
    (void)MAX30102_ReadReg(MAX30102_REG_INTR_STATUS_2);

    /* Enable A_FULL and PPG_RDY interrupts. Polling also works without INT pin. */
    MAX30102_WriteReg(MAX30102_REG_INTR_ENABLE_1, 0xC0);
    MAX30102_WriteReg(MAX30102_REG_INTR_ENABLE_2, 0x00);

    /*
     * FIFO config:
     * SMP_AVE = 4 samples averaged -> bits 7:5 = 010
     * FIFO rollover enabled -> bit 4 = 1
     * FIFO_A_FULL = 0x0F -> interrupt when 17 unread samples are available
     */
    MAX30102_WriteReg(MAX30102_REG_FIFO_CONFIG, 0x5F);

    /*
     * SpO2 config:
     * ADC range = 4096 nA -> bits 6:5 = 01
     * Sample rate = 100 sps -> bits 4:2 = 001
     * Pulse width = 411 us, 18-bit -> bits 1:0 = 11
     * Value = 0b00100111 = 0x27
     */
    MAX30102_WriteReg(MAX30102_REG_SPO2_CONFIG, 0x27);

    /*
     * LED current. 0x24 is a conservative starting point.
     * Increase slowly if IR/RED values are too low; decrease if saturated.
     */
    MAX30102_SetLEDPulseAmplitude(0x24, 0x24);

    /*
     * SpO2 mode = Red + IR, MODE[2:0] = 011.
     */
    MAX30102_WriteReg(MAX30102_REG_MODE_CONFIG, 0x03);
}

bool MAX30102_Begin(void)
{
    uint8_t id;

    id = MAX30102_GetPartID();
    if(id != MAX30102_EXPECTED_ID)
    {
        return false;
    }

    MAX30102_Reset();
    MAX30102_ClearFIFO();
    MAX30102_SetupSpO2();
    MAX30102_AlgorithmReset();

    return true;
}

uint8_t MAX30102_GetAvailableSamples(void)
{
    uint8_t wr = MAX30102_ReadReg(MAX30102_REG_FIFO_WR_PTR) & 0x1F;
    uint8_t rd = MAX30102_ReadReg(MAX30102_REG_FIFO_RD_PTR) & 0x1F;

    if(wr >= rd)
    {
        return (uint8_t)(wr - rd);
    }

    return (uint8_t)(32 + wr - rd);
}

bool MAX30102_ReadFIFO(uint32_t *red, uint32_t *ir)
{
    uint8_t data[6];

    if(!MAX30102_ReadBytes(MAX30102_REG_FIFO_DATA, data, 6))
    {
        return false;
    }

    /*
     * In SpO2 mode LED1 = Red and LED2 = IR.
     * Each channel is 18-bit left-justified in 3 bytes; upper 6 bits are unused.
     */
    *red = (((uint32_t)(data[0] & 0x03)) << 16) | ((uint32_t)data[1] << 8) | data[2];
    *ir  = (((uint32_t)(data[3] & 0x03)) << 16) | ((uint32_t)data[4] << 8) | data[5];

    return true;
}

void MAX30102_AlgorithmReset(void)
{
    sample_index = 0;

    ir_dc = 0;
    red_dc = 0;
    ir_prev2 = 0;
    ir_prev1 = 0;
    last_peak_sample = 0;
    bpm_value = 0;
    bpm_valid_state = false;

    spo2_count = 0;
    ir_min = 0xFFFFFFFFUL;
    ir_max = 0;
    red_min = 0xFFFFFFFFUL;
    red_max = 0;
    spo2_value = 0;
    spo2_valid_state = false;
}

static uint8_t clamp_u8(int16_t value, uint8_t min_value, uint8_t max_value)
{
    if(value < (int16_t)min_value) return min_value;
    if(value > (int16_t)max_value) return max_value;
    return (uint8_t)value;
}

void MAX30102_ProcessSample(uint32_t red, uint32_t ir, MAX30102_Result_t *result)
{
    bool finger;
    int32_t ir_ac;
    int32_t red_ac;
    int32_t peak_threshold;
    uint32_t interval;
    uint16_t bpm_calc;

    result->red_raw = red;
    result->ir_raw = ir;
    result->finger_detected = false;
    result->bpm = bpm_value;
    result->spo2 = spo2_value;
    result->bpm_valid = bpm_valid_state;
    result->spo2_valid = spo2_valid_state;

    sample_index++;

    finger = (ir > MAX30102_FINGER_THRESHOLD);
    result->finger_detected = finger;

    if(!finger)
    {
        MAX30102_AlgorithmReset();
        result->red_raw = red;
        result->ir_raw = ir;
        result->finger_detected = false;
        return;
    }

    if(ir_dc == 0)
    {
        ir_dc = (int32_t)ir;
        red_dc = (int32_t)red;
    }

    /*
     * DC removal using IIR filter:
     * dc = dc + (sample - dc) / 16
     */
    ir_dc += (((int32_t)ir - ir_dc) >> 4);
    red_dc += (((int32_t)red - red_dc) >> 4);

    ir_ac = (int32_t)ir - ir_dc;
    red_ac = (int32_t)red - red_dc;
    (void)red_ac;

    /*
     * Basic peak detection on the IR AC waveform.
     * A valid peak is a local maximum over threshold and not too close to the previous one.
     */
    peak_threshold = 300;

    if((ir_prev1 > ir_prev2) && (ir_prev1 > ir_ac) && (ir_prev1 > peak_threshold))
    {
        interval = sample_index - 1UL - last_peak_sample;

        if(interval > 30UL && interval < 200UL)   /* 30..200 samples at 100 Hz = 200..30 BPM */
        {
            bpm_calc = (uint16_t)((60UL * MAX30102_SAMPLE_RATE_HZ) / interval);

            if(bpm_calc >= 30U && bpm_calc <= 200U)
            {
                if(bpm_valid_state)
                {
                    bpm_value = (uint8_t)(((uint16_t)bpm_value * 3U + bpm_calc) / 4U);
                }
                else
                {
                    bpm_value = (uint8_t)bpm_calc;
                    bpm_valid_state = true;
                }
            }
        }

        last_peak_sample = sample_index - 1UL;
    }

    ir_prev2 = ir_prev1;
    ir_prev1 = ir_ac;

    /*
     * SpO2 estimate using ratio of ratios over a 1-second window:
     * R = (AC_red/DC_red) / (AC_ir/DC_ir)
     * SpO2 approx = 110 - 25*R
     *
     * This is a practical embedded approximation, not a clinical calibration.
     */
    if(ir < ir_min) ir_min = ir;
    if(ir > ir_max) ir_max = ir;
    if(red < red_min) red_min = red;
    if(red > red_max) red_max = red;

    spo2_count++;

    if(spo2_count >= MAX30102_SAMPLE_RATE_HZ)
    {
        uint32_t ir_ac_pp = ir_max - ir_min;
        uint32_t red_ac_pp = red_max - red_min;

        if(ir_ac_pp > 50UL && red_ac_pp > 50UL && ir_dc > 0 && red_dc > 0)
        {
            /*
             * ratio100 = R * 100
             * Keep it integer to avoid float cost on PIC18.
             */
            uint32_t numerator = red_ac_pp * (uint32_t)ir_dc * 100UL;
            uint32_t denominator = ir_ac_pp * (uint32_t)red_dc;

            if(denominator != 0UL)
            {
                uint32_t ratio100 = numerator / denominator;
                int16_t spo2_calc = (int16_t)(110 - (int16_t)((25UL * ratio100) / 100UL));

                spo2_value = clamp_u8(spo2_calc, 70, 100);
                spo2_valid_state = true;
            }
        }

        spo2_count = 0;
        ir_min = 0xFFFFFFFFUL;
        ir_max = 0;
        red_min = 0xFFFFFFFFUL;
        red_max = 0;
    }

    result->bpm = bpm_value;
    result->spo2 = spo2_value;
    result->bpm_valid = bpm_valid_state;
    result->spo2_valid = spo2_valid_state;
}

//=============================DS18B20==========================================

static void DS18B20_DriveLow(void)
{
    DS18B20_LAT = 0;
    DS18B20_TRIS = 0;   // salida
}

static void DS18B20_Release(void)
{
    DS18B20_TRIS = 1;   // entrada, la resistencia pull-up sube la línea
}

bool DS18B20_Start(void)
{
    bool presence;

    DS18B20_DriveLow();
    __delay_us(480);

    DS18B20_Release();
    __delay_us(70);

    presence = (DS18B20_PORT == 0);   // si responde, baja la línea´
    __delay_us(410);

    return presence;
}

static void DS18B20_WriteBit(uint8_t bit_value)
{
    DS18B20_DriveLow();

    if(bit_value)
    {
        __delay_us(6);
        DS18B20_Release();
        __delay_us(64);
    }
    else
    {
        __delay_us(60);
        DS18B20_Release();
        __delay_us(10);
    }
}

static uint8_t DS18B20_ReadBit(void)
{
    uint8_t bit_value;

    DS18B20_DriveLow();
    __delay_us(6);

    DS18B20_Release();
    __delay_us(9);

    bit_value = DS18B20_PORT;

    __delay_us(55);

    return bit_value;
}

void DS18B20_WriteByte(uint8_t data)
{
    uint8_t i;

    for(i = 0; i < 8; i++)
    {
        DS18B20_WriteBit(data & 0x01);
        data >>= 1;
    }
}

uint8_t DS18B20_ReadByte(void)
{
    uint8_t i;
    uint8_t data = 0;

    for(i = 0; i < 8; i++)
    {
        if(DS18B20_ReadBit())
        {
            data |= (1 << i);
        }
    }

    return data;
}

int16_t DS18B20_ReadRaw(void)
{
    uint8_t temp_lsb;
    uint8_t temp_msb;
    int16_t raw;

    if(!DS18B20_Start())
    {
        return 0x7FFF;   // error: sensor no detectado
    }

    DS18B20_WriteByte(0xCC);   // Skip ROM
    DS18B20_WriteByte(0x44);   // Convert T

    __delay_ms(200);           // conversión a 12 bits

    if(!DS18B20_Start())
    {
        return 0x7FFF;
    }

    DS18B20_WriteByte(0xCC);   // Skip ROM
    DS18B20_WriteByte(0xBE);   // Read Scratchpad

    temp_lsb = DS18B20_ReadByte();
    temp_msb = DS18B20_ReadByte();

    raw = ((int16_t)temp_msb << 8) | temp_lsb;

    return raw;
}

int16_t DS18B20_ReadTempC_x100(void)
{
    int16_t raw;
    int32_t temp_x100;

    raw = DS18B20_ReadRaw();

    if(raw == 0x7FFF)
    {
        return 32767;    // error
    }

    // DS18B20: 1 bit = 0.0625 °C
    // temp * 100 = raw * 6.25 = raw * 625 / 100
    temp_x100 = ((int32_t)raw * 625) / 100;

    return (int16_t)temp_x100;
}

void DS18B20_SetResolution(uint8_t resolution)
{
    uint8_t config;

    switch(resolution)
    {
        case 9:
            config = 0x1F;
            break;

        case 10:
            config = 0x3F;
            break;

        case 11:
            config = 0x5F;
            break;

        case 12:
        default:
            config = 0x7F;
            break;
    }

    if(!DS18B20_Start())
    {
        return;
    }

    DS18B20_WriteByte(0xCC); // Skip ROM
    DS18B20_WriteByte(0x4E); // Write Scratchpad

    DS18B20_WriteByte(0x00); // TH register
    DS18B20_WriteByte(0x00); // TL register
    DS18B20_WriteByte(config);
}


//============================LEDS DE ESTADO====================================
void LEDs_Init(void)
{
    LED_ENCENDIDO_TRIS = 0;
    LED_FUNCIONAL_TRIS = 0;
    LED_PREPARANDO_TRIS = 0;
    LED_ESPERA_TRIS = 0;
    LED_ALERTA_TRIS = 0;

    LED_ENCENDIDO_LAT = 0;
    LED_FUNCIONAL_LAT = 0;
    LED_PREPARANDO_LAT = 0;
    LED_ESPERA_LAT = 0;
    LED_ALERTA_LAT = 0;
}

void LEDs_Apagar_Estados(void)
{
    LED_FUNCIONAL_LAT = 0;
    LED_PREPARANDO_LAT = 0;
    LED_ESPERA_LAT = 0;
}

//==============================ALARMAS=========================================
uint8_t Verificar_Rangos(int16_t temp_x100, uint8_t bpm, uint8_t spo2, uint8_t mano_detectada)
{
    // Si no hay mano, no evaluamos alarma
    if(!mano_detectada)
    {
        return 0;
    }
    

    // Error de temperatura
    if(temp_x100 == 32767)
    {
        return 1;
    }

    // Temperatura fuera de rango
    if(temp_x100 < (TEMP_MIN * 100) || temp_x100 > (TEMP_MAX * 100))
    {
        return 1;
    }

    // BPM fuera de rango
    if(bpm < BPM_MIN || bpm > BPM_MAX)
    {
        return 1;
    }

    // SpO2 fuera de rango
    if(spo2 < SPO2_MIN)
    {
        return 1;
    }

    return 0;
}

//===========================SLEEP==========================================

void INT2_Init(void)
{
    // Configurar RB2 como entrada
    TRISBbits.TRISB2 = 1;
    
    // Configurar interrupción externa 2 (INT2)
    INTCON2bits.INTEDG2 = 1;   // Interrupción por flanco de SUBIDA (Rising Edge)
    INTCON3bits.INT2IF = 0;    // Limpiar bandera de interrupción
    INTCON3bits.INT2IE = 1;    // Habilitar la interrupción INT2
}


void Sistema_EntrarSleep(void)
{
    // Avisar por UART antes de dormir
    Uart_Send_String("Sistema entrando en modo SLEEP...\r\n");
    Uart_Send_String("SLEEP:ON\r\n");

    // Apagar LEDs de estado
    LED_ENCENDIDO_LAT = 0;
    LEDs_Apagar_Estados();
    LED_ALERTA_LAT = 0;

    // Apagar MAX30102
    MAX30102_Shutdown(true);

    // Apagar pantalla OLED
    OLED_ClearDisplay();
    OLED_Update();
    __delay_ms(500);
    OLED_DisplayOff();

    __delay_ms(2500);
    
    // --- PREPARAR DESPERTAR ---
    INTCON3bits.INT2IF = 0;    // Limpiar bandera antes de dormir
    INTCON3bits.INT2IE = 1;    // Asegurar que INT2 esté habilitada

    // Dormir PIC
    Sleep();

    // El programa continua aqui cuando despierte
    INTCON3bits.INT2IF = 0;

    // Reactivar OLED
    OLED_DisplayOn();
    OLED_ClearDisplay();
    OLED_SetFont(FONT_1);
    OLED_Write_Text_Centered(28, "Despertando...");
    OLED_Update();

    // Reactivar MAX30102
    MAX30102_Shutdown(false);

    // Reconfigurar MAX30102 por seguridad
    MAX30102_Begin();

    // Encender LED de sistema
    LED_ENCENDIDO_LAT = 1;
    LEDs_Apagar_Estados();
    LED_PREPARANDO_LAT = 1;

    Uart_Send_String("SLEEP:OFF\r\n");

    __delay_ms(1000);
}

