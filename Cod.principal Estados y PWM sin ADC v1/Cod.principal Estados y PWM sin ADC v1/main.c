/*
 * main.c
 *
 * Created: 10/24/2023 2:43:14 PM
 *  Author: • ???£?? •
 */ 

#include <xc.h>
#include <avr/interrupt.h>
#include <avr/io.h>

typedef enum{Reposo,Sensor,Reversa,Giro,Velo1,Velo2,Velo3,Velo4}estados;

estados Emi;


void ReposoF()
{
	PORTD |= (1<<DDD2); //Encendido Verde
	PORTD |= (1<<DDD4); //Encendido Rojo
	PORTB |= (0 << DDB1)|(0 << DDB2)|(0 << DDB3)|(0 << DDB4);
	//if START ON saltar a estado SensorF 
}

void SensorF()
{
	PORTD |= (1<<DDD4); //Encendido Rojo
	PORTD &= (0<<DDD2); //Apagado Verde
	//if SENSOR ON saltar a ReversaF
	Emi=Velo1;
}

void ReversaF()
{
	// Ajustar los ciclos de trabajo para OC0A (PD6) y OC0B (PD5)
	OCR0A = 0x40;  // Configuración de un ciclo de trabajo del 25% en OC0A
	OCR0B = 0x40;   // Configuración de un ciclo de trabajo del 25% en OC0B
	
	PORTB |= (0 << DDB1)|(1 << DDB2)|(0 << DDB3)|(1 << DDB4); //Motores en reversa mami
	
	// DELAY,SLEEP,MILLIS
	Emi=Giro;
}

void GiroF()
{
	
	// Ajustar los ciclos de trabajo para OC0A (PD6) y OC0B (PD5)
	OCR0A = 0x40;  // Configuración de un ciclo de trabajo del 25% en OC0A
	OCR0B = 0x40;   // Configuración de un ciclo de trabajo del 25% en OC0B
	
	PORTB |= (1 << DDB1)|(0 << DDB2)|(0 << DDB3)|(1 << DDB4); //Motores siguen girando
	
	// DELAY,SLEEP,MILLIS
	Emi=Sensor;
}

void Velo1F()
{
	PORTD |= (1<<DDD2); //Encendido Verde
	PORTD &= (0<<DDD4); //Apagado Rojo
	
	// Ajustar los ciclos de trabajo para OC0A (PD6) y OC0B (PD5)
	OCR0A = 0x40;  // Configuración de un ciclo de trabajo del 25% en OC0A
	OCR0B = 0x40;   // Configuración de un ciclo de trabajo del 25% en OC0B
	
	PORTB |= (1 << DDB1)|(0 << DDB2)|(1 << DDB3)|(0 << DDB4); //Motores adelante
	
	//Verificacion de REPOSO
	//VERIFICACION de SENSOR
	Emi=Velo2;
}

void Velo2F()
{
	// Ajustar los ciclos de trabajo para OC0A (PD6) y OC0B (PD5)
	OCR0A = 0x4D;  // Configuración de un ciclo de trabajo del 30% en OC0A
	OCR0B = 0x4D;   // Configuración de un ciclo de trabajo del 30% en OC0B
	
	PORTB |= (1 << DDB1)|(0 << DDB2)|(1 << DDB3)|(0 << DDB4); //Motores adelante
	
	//Verificacion de REPOSO
	//VERIFICACION de SENSOR
	Emi=Velo3;
}

void Velo3F()
{
	// Ajustar los ciclos de trabajo para OC0A (PD6) y OC0B (PD5)
	OCR0A = 0x59;  // Configuración de un ciclo de trabajo del 35% en OC0A
	OCR0B = 0x59;   // Configuración de un ciclo de trabajo del 35% en OC0B
	
	PORTB |= (1 << DDB1)|(0 << DDB2)|(1 << DDB3)|(0 << DDB4); //Motores adelante
	
	//Verificacion de REPOSO
	//VERIFICACION de SENSOR
	Emi=Velo4;
}

void Velo4F()
{
	// Ajustar los ciclos de trabajo para OC0A (PD6) y OC0B (PD5)
	OCR0A = 0x7F;  // Configuración de un ciclo de trabajo del 50% en OC0A
	OCR0B = 0x7F;   // Configuración de un ciclo de trabajo del 50% en OC0B
	
	PORTB |= (1 << DDB1)|(0 << DDB2)|(1 << DDB3)|(0 << DDB4); //Motores adelante
	
	//Verificacion de REPOSO
	//VERIFICACION de SENSOR
}

void PWMconfig()
{
	// Configurar Timer0 para PWM no invertido en OC0A (PD6) y OC0B (PD5)
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00) | (1 << WGM01);
	TCCR0B |= (1 << CS01);  // Establecer el preescalador a 8 para una frecuencia base

	// Configurar los pines OC0A (PD6) y OC0B (PD5) como salidas
	DDRD |= (1 << DDD6) | (1 << DDD5);
}

int main(void)
{
	DDRD &=(0<<DDD0); //PIND0 como entrada PULSADOR/START
	DDRD |= (1<<DDD2); //PIND2 como salida LedVerde
	DDRD |= (1<<DDD4); //PIND4 como salida LedRoja
	DDRD &= (0 << DDD7); //PIND7 como entrada SENSOR
	
	// Configurar como salida pines 1, 2 (pd 9 y 10 motor 1) y 3, 4 (pd 11 y 12 motor 2)
	DDRB |= (1 << DDB1)|(1 << DDB2)|(1 << DDB3)|(1 << DDB4);
	
	PWMconfig();
	
	
	while (1) 
	{
		if (PIND0==1) // Lee el valor del pin PD0
		{
			PORTD |= (1<<DDD2); //Encendido
			PORTD |= (1<<DDD4); //Encendido
		}
		else
		{
			PORTD &= (0<<DDD2); //Apagado
			PORTD &= (0<<DDD4); //Apagado
		}
	}

	return 0;
}

/*
PD 0 pulsador (pin 0d)
DDRD &=(1<<DDD0); //PIND0 como entrada PULSADOR/START
if (PIND & (1 << PIND0)) // Lee el valor del pin PD0

pd 1 led blanca

pd 2 led verde
DDRD |= (1<<DDD2) //PIND2 como salida LedVerde
PORTD |= (1<<DDD2) //Encendido
PORTD &= (0<<DDD2) //Apagado

pd 4 led rojo
DDRD |= (1<<DDD4) //PIND4 como salida LedRoja
PORTD |= (1<<DDD4) //Encendido
PORTD &= (0<<DDD4) //Apagado

pd 7 choque
DDRD &= (1 << DDD7); //PIND7 como entrada
if (PIND & (1 << PIND7)) // Lee el valor del pin PD7
*/
