
#include "Aplicacion.h"


uint8_t txStart;

void InitUART0 (void)
{
	//1.- Registro PCONP (0x400FC0C4) - bit 3 en 1 prende la UART:
	PCONP |= 0x01<<3;
	//2.- Registro PCLKSEL0 (0x400FC1A8) - bits 6 y 7 en 0 seleccionan que el clk de la UART0 sea 25MHz:
	PCLKSEL0 &= ~(0x03<<6);
	//3.- Registro U1LCR (0x4001000C) - transmision de 8 bits, 1 bit de stop, sin paridad, sin break cond, DLAB = 1:
	U0LCR = 0x00000083;
	//4.- Registros U1DLL (0x40010000) y U1DLM (0x40010004) - 115200 baudios:
	U0DLM = 0;
	U0DLL = 0xA3;//0xD9;
	//5.- Registros PINSEL0 (0x4002C000) y PINSEL1 (0x4002C004) - habilitan las funciones especiales de los pines:
	//TX1D : PIN ??	-> 		P0[2]	-> PINSEL0: 04:05
	SetPINSEL(TX0,PINSEL_FUNC1);
	//RX1D : PIN ??	-> 		P0[3]	-> PINSEL1: 06:07
	SetPINSEL(RX0,PINSEL_FUNC1);
	//6.- Registro U1LCR, pongo DLAB en 0:
	U0LCR = 0x03;
	//7. Habilito las interrupciones (En la UART -IER- y en el NVIC -ISER)
	U0IER = 0x03;
	ISER0 |= (1<<5);
}

void UART0_IRQHandler (void)
{
	uint8_t iir, dato;

	do
	{
		//IIR es reset por HW, una vez que lo lei se resetea.
		iir = U0IIR;

		if ( iir & 0x02 ) //THRE
		{
			if( !PopTx(&dato) )
				U0THR = dato;
			else
				txStart = 0;
		}
		if ( iir & 0x04 ) //Data ready
		{
			PushRx((uint8_t )U0RBR );
		}

	}
	while( ! ( iir & 0x01 ) ); /* me fijo si cuando entre a la ISR habia otra
						     	int. pendiente de atencion: b0=1 (ocurre unicamente si dentro del mismo
								espacio temporal lleguan dos interrupciones a la vez) */
}
