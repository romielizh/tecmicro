/*
 * main.c
 *
 * Created: 10/24/2023 2:43:14 PM
 *  Author: • Σπ↑£↑Θ •
 */ 
//esto es un cambio
#include <xc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
//#include <util/delay.h>

typedef enum{Reposo,Sensor,Reversa,Giro,Velo1,Velo2,Velo3,Velo4,cestados}estados;

estados Emi;

int chocaste=0;
int start_stop=0;

void StartButton()
{
	if (PINB & (1 << PINB5)) // Lee el valor del pin PB5 PULSADOR/START
	{
		start_stop=1;
	}
}

void ChoqueButton()
{
	if (PIND & (1 << PIND7)) // Lee el valor del pin PD7 CHOQUE
	{
		chocaste=1;
	}
}

void ReposoF()
{

	PORTD |= (1<<DDD2); //Encendido Verde
	PORTD |= (1<<DDD4); //Encendido Rojo
	PORTB |= (0 << DDB1)|(0 << DDB2)|(0 << DDB3)|(0 << DDB4);

	StartButton();

	if (start_stop==1)
	{
		Emi=Sensor;
		start_stop=0;
	}
}

void SensorF()
{
	PORTD |= (1<<DDD4); //Encendido Rojo
	PORTD &= (0<<DDD2); //Apagado Verde
	
	ChoqueButton();
	
	if (chocaste==1)
	{
		Emi=Reversa;
		chocaste=0;
	}
	else
	{
		Emi=Velo1;
	}
}

void ReversaF()
{
	// Ajustar los ciclos de trabajo para OC0A (PD6) y OC0B (PD5)
	OCR0A = 0x40;  // Configuración de un ciclo de trabajo del 25% en OC0A
	OCR0B = 0x40;   // Configuración de un ciclo de trabajo del 25% en OC0B
	
	PORTB |= (0 << DDB1)|(1 << DDB2)|(0 << DDB3)|(1 << DDB4); //Motores en reversa mami
	
	//_delay_ms(500);
	Emi=Giro;
}

void GiroF()
{
	
	// Ajustar los ciclos de trabajo para OC0A (PD6) y OC0B (PD5)
	OCR0A = 0x40;  // Configuración de un ciclo de trabajo del 25% en OC0A
	OCR0B = 0x40;   // Configuración de un ciclo de trabajo del 25% en OC0B
	
	PORTB |= (1 << DDB1)|(0 << DDB2)|(0 << DDB3)|(1 << DDB4); //Motores siguen girando
	
	//_delay_ms(5000);
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
	StartButton();
	if (start_stop==1)
	{
		Emi=Reposo;
		start_stop=0;
	}
	//VERIFICACION de SENSOR
	ChoqueButton();
	if (chocaste==1)
	{
		Emi=Sensor;
		chocaste=0;
	}
	
	//Emi=Velo2;
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
	DDRB &= (0<<DDB5); //PINB5 como entrada PULSADOR/START
	DDRD |= (1<<DDD2); //PIND2 como salida LedVerde
	DDRD |= (1<<DDD4); //PIND4 como salida LedRoja
	DDRD &= (0 << DDD7); //PIND7 como entrada SENSOR

	
	// Configurar como salida pines 1, 2 (pd 9 y 10 motor 1) y 3, 4 (pd 11 y 12 motor 2)
	DDRB |= (1 << DDB1)|(1 << DDB2)|(1 << DDB3)|(1 << DDB4);
	
	PWMconfig();
	
	void(*func[cestados])();
	
	func[Reposo]=ReposoF;

	func[Sensor]=SensorF;

	func[Reversa]=ReversaF;
  
	func[Giro]=GiroF;
  
	func[Velo1]=Velo1F;
  
	func[Velo2]=Velo2F;
  
	func[Velo3]=Velo3F;
  
	func[Velo4]=Velo4F;
  
	Emi=Reposo;
	
	while (1) 
	{
		func[Emi]();
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


/*
		if (PIND & (1 << PIND7)) // Lee el valor del pin PD0
		{
			PORTD &= (0<<DDD2); //Apagado
			PORTD &= (0<<DDD4); //Apagado
		}
		else
		{
			PORTD |= (1<<DDD2); //Encendido
			PORTD |= (1<<DDD4); //Encendido
		}
*/