/*
 * main.c
 *
 * Created: 10/24/2023 2:43:14 PM
 *  Author: • Σπ↑£↑Θ •
 */ 

#include <xc.h>
#include <avr/interrupt.h>
#include <avr/io.h>


void setupPWM() 
{
	// Configurar Timer0 para PWM no invertido en OC0A (PD6) y OC0B (PD5)
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00) | (1 << WGM01);
	TCCR0B |= (1 << CS01);  // Establecer el preescalador a 8 para una frecuencia base

	// Configurar los pines OC0A (PD6) y OC0B (PD5) como salidas
	DDRD |= (1 << DDD6) | (1 << DDD5);
}

int main(void)
{

	// Configurar como salida pines 1, 2 (pd 9 y 10 motor 1) y 3, 4 (pd 11 y 12 motor 2)
	DDRB |= (1 << DDB1)|(1 << DDB2)|(1 << DDB3)|(1 << DDB4);
	
	// Control de avance retroceso motores
	PORTB |= (1 << DDB1)|(0 << DDB2)|(1 << DDB3)|(0 << DDB4);
	
	setupPWM();

	// Ajustar los ciclos de trabajo para OC0A (PD6) y OC0B (PD5)
	OCR0A = 0xFF;  // Configuración de un ciclo de trabajo del 100% en OC0A
	OCR0B = 0xFF;   // Configuración de un ciclo de trabajo del 100% en OC0B

	
	
	while (1) 
	{
				
			
			
	}

	return 0;
}