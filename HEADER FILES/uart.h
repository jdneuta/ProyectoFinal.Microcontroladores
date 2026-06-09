/* 
 * File:   uart.h
 * Author: María De Los Ángeles Castillo 
 *
 * Created on 25 de mayo de 2026, 07:06 PM
 */

#ifndef UART_H
#define	UART_H

#include <xc.h>
#include "config.h"

void Uart_Init(unsigned long baud);
void Uart_Send_Char(char txData);
void Uart_Send_String(char *info);

#endif	/* UART_H */

