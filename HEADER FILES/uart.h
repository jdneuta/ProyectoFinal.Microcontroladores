/* 
 * File:   uart.h
 * Author: MARIA DE LOS ANGELES
 *
 * Created on 21 de mayo de 2026, 17:10
 */
#ifndef UART_H
#define UART_H

#define _XTAL_FREQ  8000000UL
#define BAUD_RATE   9600


void UART_Init(void);
void UART_SendChar(char c);
void UART_SendString(const char *str);
void UART_SendUInt(unsigned int valor);
void UART_SendDecimal(unsigned int entero, unsigned int decimal);
char UART_ReadChar(void);
unsigned char UART_DataReady(void);

#endif