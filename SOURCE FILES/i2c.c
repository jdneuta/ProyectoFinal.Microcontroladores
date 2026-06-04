
#include "i2c.h"
#include "config.h"

#ifdef I2C_MASTER_MODE

void I2C_Init_Master(unsigned char sp_i2c)
{
    TRIS_SCL = 1;                   //SCL como entrada
    TRIS_SDA = 1;                   //SDA como entrada

    SSPSTAT = sp_i2c;               //Slew rate y velocidad    

    SSPCON1 = 0x28;                 // Habilita el modulo MSSP en modo I2C maestro
    SSPCON2 = 0x00;                 // Limpia registros de control adicionales del MSSP
    PIR1bits.SSPIF = 0;             // Limpia la bandera de interrupcion SSP

    /** Define el divisor de baud rate segun la velocidad solicitada. */
    if(sp_i2c == I2C_100KHZ)
    {
        SSPADD = 19;
    }
    else if(sp_i2c == I2C_400KHZ)
    {
        SSPADD = 4;
    }
}

void I2C_Start(void)
{
    PIR1bits.SSPIF = 0;
    SSPCON2bits.SEN = 1;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}


void I2C_Stop(void)
{
    PIR1bits.SSPIF = 0;
    SSPCON2bits.PEN = 1;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}

void I2C_Restart(void)
{
    PIR1bits.SSPIF = 0;
    SSPCON2bits.RSEN = 1;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}

void I2C_Ack(void)
{
    PIR1bits.SSPIF = 0;
    SSPCON2bits.ACKDT = 0;
    SSPCON2bits.ACKEN = 1;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}

void I2C_Nack(void)
{
    PIR1bits.SSPIF = 0;
    SSPCON2bits.ACKDT = 1;
    SSPCON2bits.ACKEN = 1;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}

short I2C_Write(char data)
{
    PIR1bits.SSPIF = 0;
    SSPBUF = data;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
    return SSPCON2bits.ACKSTAT;
}

unsigned char I2C_Read(void)
{
    PIR1bits.SSPIF = 0;
    SSPCON2bits.RCEN = 1;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
    return SSPBUF;
}
#endif

#ifdef I2C_SLAVE_MODE

void I2C_Init_Slave(unsigned char add_slave)
{
    TRIS_SCL = 1;
    TRIS_SDA = 1;
    SSPSTAT = 0x80;
    SSPADD = add_slave;
    SSPCON1 = 0x36;
    SSPCON2 = 0x01;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIR1bits.SSPIF = 0;
    PIE1bits.SSPIE = 1;
}


short I2C_Error_Read(void)
{
    SSPCON1bits.CKP = 0;
    return ((SSPCON1bits.SSPOV) || (SSPCON1bits.WCOL)) ? 1 : 0;
}


short I2C_Write_Mode(void)
{
    return(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) ? 1 : 0;
}

short I2C_Read_Mode(void)
{
    return (!SSPSTATbits.D_nA && SSPSTATbits.R_nW) ? 1 : 0;
}

void I2C_Error_Data(void)
{
    short z;
    SSPCON1bits.CKP = 0;
    z = SSPBUF;
    SSPCON1bits.SSPOV = 0;
    SSPCON1bits.WCOL = 0;
    SSPCON1bits.CKP = 1;
    SSPCON1bits.SSPM3 = 0;
    (void)z;
}

unsigned char I2C_Read_Slave(void)
{
    short z;
    unsigned char dato_i2c;
    z = SSPBUF;
    while(!BF)
    {
        ;
    }
    dato_i2c = SSPBUF;
    SSPCON1bits.CKP = 1;
    SSPCON1bits.SSPM3 = 0;
    (void)z;
    return dato_i2c;
}


void I2C_Write_Slave(char dato_i2c)
{
    short z;
    z = SSPBUF;
    BF = 0;
    SSPBUF = dato_i2c;
    SSPCON1bits.CKP = 1;
    while(SSPSTATbits.BF == 1)
    {
        ;
    }
    (void)z;
}
#endif
