#include "uart.h"

void Uart_Init(unsigned long baud)                  // Inicialzia el modulo USART
{
    TRISCbits.RC6 = 0;                              // Pin TX como salida
    TRISCbits.RC7 = 1;                              // Pin RX como entrada
    TXSTA = 0x24;                                   // Habilita 5(TX),2(valocidad)
    RCSTA = 0x90;                                   // Habilita 7(Serial),4(Continuo)
    BAUDCON = 0x00;                                 // Limpia el registro de baudios
    BAUDCONbits.BRG16 = 1;                          // Baudios a 16 bits
    unsigned int vx = (_XTAL_FREQ/(baud*4))-1;      // Formula para que siempre este en 9600 B
    SPBRG = vx & 0x00FF;                            // Carga los baudios de la parte baja
    SPBRGH = vx >> 8;                               // Carga los baudios de la parte alta
}

void Uart_Send_Char(char txData)         			// Funcion para transmitir caracteres
{
    while(TXSTAbits.TRMT == 0);                     //Espera que termine de enviar
    TXREG = txData;                                 //Registro de TX 
}

void Uart_Send_String(char *info)          			// Funcion para transmitir una cadena de caracteres
{
    while(*info)
    {
        Uart_Send_Char(*info++);
    }
}

