/* 
 * File:   i2c.h
 * Author: MARIA DE LOS ANGELES
 *
 * Created on 21 de mayo de 2026, 16:54
 */

#ifndef I2C_H
#define I2C_H

#include <xc.h>

#define TRIS_SCL TRISBbits.TRISB1           
#define TRIS_SDA TRISBbits.TRISB0          

#define I2C_100KHZ 0x80                     // Bus para 100 kHz
#define I2C_400KHZ 0x00                     // Bus para 100 kHz

#define I2C_MASTER_MODE                     // Habilita modo I2C 



#ifdef I2C_MASTER_MODE
void I2C_Init_Master(unsigned char sp_i2c);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Restart(void);
void I2C_Ack(void);
void I2C_Nack(void);
unsigned char I2C_Read(void);
short I2C_Write(char data);
#endif

#ifdef I2C_SLAVE_MODE
void I2C_Init_Slave(unsigned char add_slave);
unsigned char I2C_Read_Slave(void);
void I2C_Write_Slave(char dato_i2c);
void I2C_Error_Data(void);
short I2C_Write_Mode(void);
short I2C_Read_Mode(void);
short I2C_Error_Read(void);
#endif

#endif

