//?????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>									//>interupciones<

//?????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//
int Auxb1 = 0;												//>interupciones<
int Auxb2 = 0;												//>interupciones<
																							// Pines conectados a los botones
#define BOTON1_PIN PIND2									//>interupciones<
#define BOTON2_PIN PIND3									//>interupciones<

#define LED_PIN PINB4										//>led<							// Pin de salida para led



																							// Pines conectados al puente HX
#define XIN1 PINB0											//>motor1<
#define XIN2 PINB1											//>motor1<
#define XIN3 PINB2											//>motor1<
#define XIN4 PINB3											//>motor1<

																							// Pines conectados al puente HY
#define YIN1 PIND4											//>motor2<
#define YIN2 PIND5											//>motor2<
#define YIN3 PIND6											//>motor2<
#define YIN4 PIND7											//>motor2<

//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//
void ConfigInterrupciones()
{															//>interupciones<
	EICRA |= (1 << ISC01);																	// Configurar boton1 para flanco de bajada
	EICRA |= (1 << ISC11);																	// Configurar boton2 para flanco de bajada

	EIMSK |= (1 << INT0) | (1 << INT1);														// Habilitar boton1 y boton2

	sei();																					// Habilitar interrupciones globales
}															//>interupciones<

//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//

void setStepX(int step)
{															//>motor1<
																							// Tabla de pasos
	int sequence[8][4] =																	//[filas][columnas]
	{
		{0, 0, 0, 1},
		{0, 0, 1, 1},
		{0, 0, 1, 0},
		{0, 1, 1, 0},
		{0, 1, 0, 0},
		{1, 1, 0, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 1}
	};														//>motor1<

																							// Configura los pines del puente HX según la tabla
	for (int i = 0; i < 4; i++)																// [columnas]
	{
		if (sequence[step][i])
		{													//>motor1<
																							// Configura el pin en alto
			switch (i)
			{
				case 0:
				PORTB |= (1 << XIN1);
				break;
				case 1:
				PORTB |= (1 << XIN2);
				break;
				case 2:
				PORTB |= (1 << XIN3);
				break;
				case 3:
				PORTB |= (1 << XIN4);
				break;
			}
		}													//>motor1<
		else
		{													//>motor1<
																							// Configura el pin en bajo
			switch (i)
			{
				case 0:
				PORTB &= ~(1 << XIN1);
				break;
				case 1:
				PORTB &= ~(1 << XIN2);
				break;
				case 2:
				PORTB &= ~(1 << XIN3);
				break;
				case 3:
				PORTB &= ~(1 << XIN4);
				break;
			}
		}
	}
}															//>motor1<

//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//

void setStepY(int step)
{															//>motor2<
																							// Tabla de pasos
	int sequence[8][4] =																	//[filas][columnas]
	{
		{1, 0, 0, 0},
		{1, 1, 0, 0},
		{0, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 1, 0},
		{0, 0, 1, 1},
		{0, 0, 0, 1},
		{1, 0, 0, 1}
	};														//>motor2<

																							// Configura los pines del puente HY según la tabla
	for (int i = 0; i < 4; i++)																//[columnas]
	{
		if (sequence[step][i])
		{													//>motor2<
																							// Configura el pin en alto
			switch (i)
			{
				case 0:
				PORTD |= (1 << YIN1);
				break;
				case 1:
				PORTD |= (1 << YIN2);
				break;
				case 2:
				PORTD |= (1 << YIN3);
				break;
				case 3:
				PORTD |= (1 << YIN4);
				break;
			}
		}													//>motor2<
		else
		{													//>motor2<
																							// Configura el pin en bajo
			switch (i)
			{
				case 0:
				PORTD &= ~(1 << YIN1);
				break;
				case 1:
				PORTD &= ~(1 << YIN2);
				break;
				case 2:
				PORTD &= ~(1 << YIN3);
				break;
				case 3:
				PORTD &= ~(1 << YIN4);
				break;
			}
		}
	}
}															//>motor2<

//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//

int main(void)
{
															//>motor1<
	DDRB |= (1 << XIN1) | (1 << XIN2) | (1 << XIN3) | (1 << XIN4);							// Configura los pines de control del puente HX como salidas
															//>motor1<
															
															//>motor2<
	DDRD |= (1 << YIN1) | (1 << YIN2) | (1 << YIN3) | (1 << YIN4);							// Configura los pines de control del puente HY como salidas
															//>motor2<

															//>interupciones<
	DDRD &= ~((1 << BOTON1_PIN) | (1 << BOTON2_PIN));										// Configurar pines de entrada para los botones

	ConfigInterrupciones();																	// Llamada a la funcion configinterupciones
															//>interupciones<

															//>led<
	DDRB |= (1 << LED_PIN);																	// Configura pin led como salida
															//>led<


	while (1)
	{
		while (Auxb1==1)
			for (int i = 0; i < 8; i++)																// Girar en sentido horario
			{
				setStepX(i);
				setStepY(i);
				_delay_ms(50);																		// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
			}	
		
	    PORTB |= (1 << LED_PIN);
	    
	    _delay_ms(5000);
	    
	    PORTB &= ~(1 << LED_PIN);
	    
	    _delay_ms(5000);
		
		while (Auxb2==1)
			for (int i = 8; i >= 0; i--)															// Girar en sentido antihorario
			{
				setStepX(i);
				setStepY(i);
				_delay_ms(50);																		// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
			}
	}

	return 0;
}
//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//

																							// Rutina de servicio de interrupción para boton1
ISR(INT0_vect)												//>interupciones<
{
	_delay_ms(30);
	if (Auxb1 == 1)
	{
		Auxb1 = 0;
	}
	else
	{
		Auxb1 = 1;
	}
	_delay_ms(30);
}															//>interupciones<

																							// Rutina de servicio de interrupción para boton2
ISR(INT1_vect)												//>interupciones<
{
	_delay_ms(30);
	if (Auxb2 == 1)
	{
		Auxb2 = 0;
	}
	else
	{
		Auxb2 = 1;
	}
	_delay_ms(30);
}															//>interupciones<





/*
-------------------------------------
						>tabla 1-1<
		{1, 0, 1, 0},
		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 1, 1, 0}
						>tabla 1-2<
		{0, 1, 0, 1},
		{1, 0, 0, 1},
		{1, 0, 1, 0},
		{0, 1, 1, 0}
-------------------------------------
						>tabla 2-1<
		{1, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 1, 1},
		{1, 0, 0, 1}

		{0, 0, 1, 1},
		{0, 1, 1, 0},
		{1, 1, 0, 0},
		{1, 0, 0, 1}
-------------------------------------
						>tabla 1-1<
		{0, 1, 0, 1},
		{0, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 1, 0},
		{1, 0, 1, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 1},
		{0, 0, 0, 1}
						>tabla 1-2<
		{1, 0, 1, 0},
		{0, 0, 1, 0},
		{0, 1, 1, 0},
		{0, 1, 0, 0},
		{0, 1, 0, 1},
		{0, 0, 0, 1},
		{1, 0, 0, 1},
		{1, 0, 0, 0}
-------------------------------------
						>tabla 2-1<
		{1, 0, 0, 0},
		{1, 1, 0, 0},
		{0, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 1, 0},
		{0, 0, 1, 1},
		{0, 0, 0, 1},
		{1, 0, 0, 1}
						>tabla 2-2<
		{0, 0, 0, 1},
		{0, 0, 1, 1},
		{0, 0, 1, 0},
		{0, 1, 1, 0},
		{0, 1, 0, 0},
		{1, 1, 0, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 1}
-------------------------------------
*/