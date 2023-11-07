/*
 * main.c
 *
 * Created: 10/24/2023 2:43:14 PM
 *  Author: César, Emilio, Romina
 */ 

//*************************************************************************************************************************************************//

#include <xc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

//#define F_CPU 16000000UL

//****************************************************---DECLARACION DE FUNCIONES (ESTADOS)---*****************************************************//

void pinConfig();
void PWMconfig();

uint16_t ADC_read();
void usarADC();

void ReposoF();
void SensorF();
void ReversaF();
void GiroF();
void Velo1F();
void Velo2F();
void Velo3F();
void Velo4F();


//*************************************************************---VARIABLES GLOBALES---************************************************************//

int start_stop = 0;
uint8_t choques = 0;
uint16_t adress = 0;


//******************************************************************---ESTADOS---******************************************************************//

typedef enum {REPOSO,SENSOR,REVERSA,GIRO,VELO1,VELO2,VELO3,VELO4,CESTADOS} ESTADOS;  //definir un tipo de dato enum de nombre ESTADOS

void (*func[CESTADOS])() = {ReposoF, SensorF, ReversaF, GiroF, Velo1F, Velo2F, Velo3F, Velo4F}; // Puntero a los estados


//**********************************************************---CONFIGURACION DE PINES---***********************************************************//

void pinConfig() 
{
	//ADC config (antes funcion ADC_init)
	uint16_t adc_result;
	ADMUX = (1<<REFS0) | Pin;
	ADCSRA = (1<<ADEN)|(7<<ADPS0);
	DDRD |= (1<<PD1);
	//ADC config (antes funcion ADC_init)
	
	DDRB &= (0<<DDB5);			//PINB5 como entrada PULSADOR/START
	DDRD |= (1<<DDD2);			//PIND2 como salida LedVerde
	DDRD |= (1<<DDD4);			//PIND4 como salida LedRoja
	DDRD &= (0 << DDD7);		//PIND7 como entrada SENSOR

	// Configurar como salida pines b1, b2 (pd 9 y 10 motor 1) y b3, b4 (pd 11 y 12 motor 2)
	DDRB |= (1 << DDB1)|(1 << DDB2)|(1 << DDB3)|(1 << DDB4);
	
	ESTADOS actualestado = REPOSO;
}

void PWMconfig()
{
	// Configurar Timer0 para PWM no invertido en OC0A (PD6) y OC0B (PD5)
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00) | (1 << WGM01);
	TCCR0B |= (1 << CS01);  // Establecer el preescalador a 8 para una frecuencia base

	// Configurar los pines OC0A (PD6) y OC0B (PD5) como salidas
	DDRD |= (1 << DDD6) | (1 << DDD5);
}


//********************************************************************---ADC---********************************************************************//

#define Umbral 630 //ejemplo lo tenemos que variar
#define Pin 0 // pin

uint16_t ADC_read()
{
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	return (ADC);
}

void usarADC()
{
	adc_result = ADC_read();
	if(adc_result > Umbral)
		PORTD |= (1<<PD1);
	else
		PORTD &= ~(1<<PD1);
}


//************************************************************---FUNCIONES (ESTADOS)---************************************************************//

void StartButton()
{
	if (PINB & (1 << PINB5))									// Lee el valor del pin PB5 PULSADOR/START
	{
		start_stop=1;
	}
}

void ChoqueButton()
{
	if (PIND & (1 << PIND7))									// Lee el valor del pin PD7 CHOQUE
	{
		actualestado=SENSOR;
		break;
	}
}

void ReposoF()
{
	//encender todas las luces y detener motores, verificar variable global de start/stop, reset variable global antes de saltar
	
	PORTD |= (1<<PD1);											//ledBlancaON
	PORTD |= (1<<DDD2);											//ledVerdeON
	PORTD |= (1<<DDD4);											//ledRojoON
	PORTB |= (0 << DDB1)|(0 << DDB2)|(0 << DDB3)|(0 << DDB4);	//motoresOFF
	start_stop = 0;
	
	while (1)
	{
		StartButton();											//verifica pulsador on/off
		if (start_stop == 1)									//verificar variable global de start/stop
		{
			start_stop = 0;										//reset variable global
			actualestado=SENSOR;								//salta a sensor
			break;
		}
	}
}

void SensorF()
{
	//motoresOFF, ledVerdeOFF, ledRojoON
	//verificar pulsador choque, si->reversa, no->velo1
	//verificar variable global de start/stop, si->reposo, no->hacer nada ----->(no implementado, no hay while, poco tiempo de ejecucion)
	
	PORTD &= (0<<DDD2);											//ledVerdeOFF
	PORTD |= (1<<DDD4);											//ledRojoON
	PORTB |= (0 << DDB1)|(0 << DDB2)|(0 << DDB3)|(0 << DDB4);	//motoresOFF
	
	if (PIND & (1 << PIND7))									// Lee el valor del pin PD7 CHOQUE
	{
		actualestado=REVERSA;
	}
	else
	{
		actualestado=VELO1;
	}
}

void ReversaF()
{
	//set velocidad (PWM) 25% (0x40)
	//motoresON reversa (0110), led verdeOFF, led rojoON
	//esperar tiempo en while para poder verificar variable global de start/stop, si->reposo, no->hacer nada
	//verificar ADC
	//termina while motoresOFF salta giro
	
	choques++;
	
	//set velocidad (PWM) 25% (0x40)
	OCR0A = 0x40;												// Configuración de un ciclo de trabajo del 25% en OC0A
	OCR0B = 0x40;												// Configuración de un ciclo de trabajo del 25% en OC0B
	
	PORTD &= (0<<DDD2);											//ledVerdeOFF
	PORTD |= (1<<DDD4);											//ledRojoON
	PORTB |= (0 << DDB1)|(1 << DDB2)|(1 << DDB3)|(0 << DDB4);	//motoresON reversa (0110)
	
	uint16_t tiempo_espera = 500;								//tiempo en milisegundos
	
	while (tiempo_espera>0)
	{
		usarADC();												//verificar ADC
		StartButton();											//verificar start/stop
		_delay_ms(1);
		tiempo_espera--;
		if (start_stop == 1)									//verificar variable global de start/stop
		{
			actualestado=REPOSO;
			break;
		}
	}
	if (tiempo_espera == 0)
	{
		actualestado=GIRO;
	}
	PORTB |= (0 << DDB1)|(0 << DDB2)|(0 << DDB3)|(0 << DDB4);	//motoresOFF
}

void GiroF()
{
	//set velocidad (PWM) 25%
	//motoresON giro (1010/0101), led verdeOFF, led rojoON
	//esperar tiempo en while para poder verificar variable global de start/stop, si->reposo, no->hacer nada
	//verificar ADC
	//termina while motoresOFF salta sensor
	
	//set velocidad (PWM) 25% (0x40)
	OCR0A = 0x40;												// Configuración de un ciclo de trabajo del 25% en OC0A
	OCR0B = 0x40;												// Configuración de un ciclo de trabajo del 25% en OC0B
	
	PORTD &= (0<<DDD2);											//ledVerdeOFF
	PORTD |= (1<<DDD4);											//ledRojoON
	PORTB |= (1 << DDB1)|(0 << DDB2)|(1 << DDB3)|(0 << DDB4);	//motoresON giro (1010)
	
	uint16_t tiempo_espera = 5000;								//tiempo en milisegundos
	
	while (tiempo_espera>0)
	{
		usarADC();												//verificar ADC
		StartButton();											//verificar start/stop
		_delay_ms(1);
		tiempo_espera--;
		if (start_stop == 1)									//verificar variable global de start/stop
		{
			actualestado=REPOSO;
			break;
		}
	}
	if (tiempo_espera == 0)
	{
		actualestado=SENSOR;
	}
	PORTB |= (0 << DDB1)|(0 << DDB2)|(0 << DDB3)|(0 << DDB4);	//motoresOFF
}

void Velo1F()
{
	//set velocidad (PWM) 25%
	//motoresON avanza (1001), led verdeON, led rojoOFF
	//esperar tiempo while para poder verificar variable global de start/stop, si->reposo, no->hacer nada
	//verificar ADC
	//verificar variable global de pulsador choque, si->saltar sensor, no->hacer nada
	//termina while salta velo2
	
	//set velocidad (PWM) 25% (0x40)
	OCR0A = 0x40;												// Configuración de un ciclo de trabajo del 25% en OC0A
	OCR0B = 0x40;												// Configuración de un ciclo de trabajo del 25% en OC0B
	
	PORTD |= (1<<DDD2);											//ledVerdeON
	PORTD &= (0<<DDD4);											//ledRojoOFF
	PORTB |= (1 << DDB1)|(0 << DDB2)|(0 << DDB3)|(1 << DDB4);	//motoresON avanza (1001)
	
	uint16_t tiempo_espera = 5000;								//tiempo en milisegundos
	
	while (tiempo_espera>0)
	{
		usarADC();												//verificar ADC
		StartButton();											//verificar start/stop
		_delay_ms(1);
		tiempo_espera--;
		if (start_stop == 1)									//verificar variable global de start/stop
		{
			actualestado=REPOSO;
			break;
		}
		ChoqueButton();											//salta a SENSOR si pulsador choque activado
	}
	if (tiempo_espera == 0)										//si pulsador choque o pulsador start/stop activados no entra
	{
		actualestado=VELO2;
	}
	PORTB |= (0 << DDB1)|(0 << DDB2)|(0 << DDB3)|(0 << DDB4);	//motoresOFF
}

void Velo2F()
{
	//set velocidad (PWM) 35%
	//motoresON avanza (1001), led verdeON, led rojoOFF
	//esperar tiempo while para poder verificar variable global de start/stop, si->reposo, no->hacer nada
	//verificar ADC
	//verificar variable global de pulsador choque, si->saltar sensor, no->hacer nada
	//termina while salta velo2
	
	//set velocidad (PWM) 35% (0x59)
	OCR0A = 0x59;												// Configuración de un ciclo de trabajo del 35% en OC0A
	OCR0B = 0x59;												// Configuración de un ciclo de trabajo del 35% en OC0B
	
	PORTD |= (1<<DDD2);											//ledVerdeON
	PORTD &= (0<<DDD4);											//ledRojoOFF
	PORTB |= (1 << DDB1)|(0 << DDB2)|(0 << DDB3)|(1 << DDB4);	//motoresON avanza (1001)
	
	uint16_t tiempo_espera = 5000;								//tiempo en milisegundos
	
	while (tiempo_espera>0)
	{
		usarADC();												//verificar ADC
		StartButton();											//verificar start/stop
		_delay_ms(1);
		tiempo_espera--;
		if (start_stop == 1)									//verificar variable global de start/stop
		{
			actualestado=REPOSO;
			break;
		}
		ChoqueButton();											//salta a SENSOR si pulsador choque activado
	}
	if (tiempo_espera == 0)										//si pulsador choque o pulsador start/stop activados no entra
	{
		actualestado=VELO3;
	}
	PORTB |= (0 << DDB1)|(0 << DDB2)|(0 << DDB3)|(0 << DDB4);	//motoresOFF
}

void Velo3F()
{
	//set velocidad (PWM) 50%
	//motoresON avanza (1001), led verdeON, led rojoOFF
	//esperar tiempo while para poder verificar variable global de start/stop, si->reposo, no->hacer nada
	//verificar ADC
	//verificar variable global de pulsador choque, si->saltar sensor, no->hacer nada
	//termina while salta velo2
	
	//set velocidad (PWM) 50% (0x80)
	OCR0A = 0x80;												// Configuración de un ciclo de trabajo del 50% en OC0A
	OCR0B = 0x80;												// Configuración de un ciclo de trabajo del 50% en OC0B
	
	PORTD |= (1<<DDD2);											//ledVerdeON
	PORTD &= (0<<DDD4);											//ledRojoOFF
	PORTB |= (1 << DDB1)|(0 << DDB2)|(0 << DDB3)|(1 << DDB4);	//motoresON avanza (1001)
	
	uint16_t tiempo_espera = 5000;								//tiempo en milisegundos
	
	while (tiempo_espera>0)
	{
		usarADC();												//verificar ADC
		StartButton();											//verificar start/stop
		_delay_ms(1);
		tiempo_espera--;
		if (start_stop == 1)									//verificar variable global de start/stop
		{
			actualestado=REPOSO;
			break;
		}
		ChoqueButton();											//salta a SENSOR si pulsador choque activado
	}
	if (tiempo_espera == 0)										//si pulsador choque o pulsador start/stop activados no entra
	{
		actualestado=VELO4;
	}
	PORTB |= (0 << DDB1)|(0 << DDB2)|(0 << DDB3)|(0 << DDB4);	//motoresOFF
}

void Velo4F()
{
	//set velocidad (PWM) 100%
	//motoresON avanza (1001), led verdeON, led rojoOFF
	//esperar tiempo while para poder verificar variable global de start/stop, si->reposo, no->hacer nada
	//verificar ADC
	//verificar variable global de pulsador choque, si->saltar sensor, no->hacer nada
	//termina while salta velo2
	
	//set velocidad (PWM) 100% (0xFF)
	OCR0A = 0xFF;												// Configuración de un ciclo de trabajo del 100% en OC0A
	OCR0B = 0xFF;												// Configuración de un ciclo de trabajo del 100% en OC0B
	
	PORTD |= (1<<DDD2);											//ledVerdeON
	PORTD &= (0<<DDD4);											//ledRojoOFF
	PORTB |= (1 << DDB1)|(0 << DDB2)|(0 << DDB3)|(1 << DDB4);	//motoresON avanza (1001)
	
	while (1)
	{
		usarADC();												//verificar ADC
		StartButton();											//verificar start/stop
		_delay_ms(1);
		if (start_stop == 1)									//verificar variable global de start/stop
		{
			actualestado=REPOSO;
			break;
		}
		ChoqueButton();											//salta a SENSOR si pulsador choque activado
	}
	PORTB |= (0 << DDB1)|(0 << DDB2)|(0 << DDB3)|(0 << DDB4);	//motoresOFF
}


//******************************************************************---INICIO---*******************************************************************//

int main() 
{
	pinConfig();
	PWMconfig();
	
	while (1) 
	{
		// Llamar a la función del estado actual
		(func[actualestado])();
		
		// Verificar led blanca
		usarADC();
		
		//Verificar pulsador start/stop
		//actualestado=REPOSO;
		
	}
	return 0;
}

//********************************************************************---FIN---********************************************************************//