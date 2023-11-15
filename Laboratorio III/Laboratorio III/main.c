//?????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>									//>interupciones<

//?????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//

#define F_CPU 16000000UL									//>UART<
#define BAUD 9600											//>UART<
#define MYUBRR F_CPU/16/BAUD-1								//>UART<
																							// Pines conectados a los botones
#define BOTON1_PIN PIND2									//>interupciones<
#define BOTON2_PIN PIND3									//>interupciones<
unsigned int Aux1 = 0;										//>interupciones<
unsigned int Aux2 = 0;										//>interupciones<
																							// Pin de salida para solenoide
#define SOL_PIN PINB4										//>solenoide<
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
																							// Estados para los dibujos y seleccion de opciones
typedef enum 
{															//>UART<
	ESPERAR,
	ESTADO_OPCION_1,
	ESTADO_OPCION_2,
	ESTADO_OPCION_3,
	ESTADO_OPCION_4,
	MODO_LIBRE
} State;													//>UART<
																							// Seleccion de estados para opciones de dibujo
volatile State state = ESPERAR;								//>UART<
volatile int selectedOption = 0;							//>UART<

unsigned int PASOS = 0;

//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//
//>Configuraciones<
void ConfigInterrupciones()
{															//>interupciones<
	EICRA |= (1 << ISC01);																	// Configurar boton1 para flanco de bajada
	EICRA |= (1 << ISC11);																	// Configurar boton2 para flanco de bajada
	EIMSK |= (1 << INT0) | (1 << INT1);														// Habilitar boton1 y boton2
	sei();																					// Habilitar interrupciones globales
}															//>interupciones<

void USART_Init(unsigned int ubrr) 
{															//>UART<
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit(unsigned char data)
{
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

unsigned char USART_Receive(void)
{
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}															//>UART<

//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//
//>Secuencia_pasos_X<
void setStepX(int step)
{															//>motor1<
																							// Tabla de pasos
	int sequence[8][4] =																	//[filas][columnas]
	{
		{1, 0, 0, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 0}
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
//>Secuencia_pasos_Y<
void setStepY(int step)
{															//>motor2<
																							// Tabla de pasos
	int sequence[8][4] =																	//[filas][columnas]
	{
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
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
//>Secuencia_ejecucion_dibujos_UART<
void handle_state()
{															//>UART<
	switch (state)
	{
		case ESPERAR:
			break;
		case ESTADO_OPCION_1:
			USART_Transmit('1');
			USART_Transmit('e');
			USART_Transmit('f');
			USART_Transmit('\n');
			state = ESPERAR;
			break;
		case ESTADO_OPCION_2:
			USART_Transmit('2');
			USART_Transmit('e');
			USART_Transmit('f');
			USART_Transmit('\n');
			state = ESPERAR;
			break;
		case ESTADO_OPCION_3:
			USART_Transmit('3');
			USART_Transmit('e');
			USART_Transmit('f');
			USART_Transmit('\n');
			state = ESPERAR;
			break;
		case ESTADO_OPCION_4:
			USART_Transmit('4');
			USART_Transmit('e');
			USART_Transmit('f');
			USART_Transmit('\n');
			state = ESPERAR;
			break;
		case MODO_LIBRE:
			USART_Transmit('l');
			USART_Transmit('e');
			USART_Transmit('l');
			USART_Transmit('\n');
			state = ESPERAR;
			break;
	}
}

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

															//>solenoide<
	DDRB |= (1 << SOL_PIN);																	// Configura pin solenoide como salida
															//>solenoide<

															//>UART<
    USART_Init(MYUBRR);																		// Configura UART
															//>UART<

	while (1)
	{
		handle_state();
		/*//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		while (Auxb1==1)
			for (int i = 0; i < 8; i++)														// Girar derechaY abajoX
			{
				setStepX(i);
				setStepY(i);
				_delay_ms(150);																// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
			}
		
		//while (Auxb2==1)
		for (int i = 8; i >= 0; i--)														// Girar en sentido antihorario
		{
			setStepX(i);
			setStepY(i);
			_delay_ms(150);																	// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
		}
		
		*///>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		
		/*//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	    PORTB |= (1 << SOL_PIN);							//>solenoide<
	    
	    _delay_ms(2000);
	    
	    PORTB &= ~(1 << SOL_PIN);
	    
	    _delay_ms(2000);									//>solenoide<
		*///>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	}

	return 0;
}
//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//

																							// Rutina de servicio de interrupción para boton1
ISR(INT0_vect)
{															//>interupciones<
	
	_delay_ms(1000);
	selectedOption = (selectedOption + 1) % 5;
	switch (selectedOption)
	{
		case 0:
		USART_Transmit('1');
		USART_Transmit(':');
		USART_Transmit(' ');
		USART_Transmit('D');
		USART_Transmit('i');
		USART_Transmit('b');
		USART_Transmit('u');
		USART_Transmit('j');
		USART_Transmit('o');
		USART_Transmit(' ');
		USART_Transmit('1');
		USART_Transmit('\n');
		break;
		case 1:
		USART_Transmit('2');
		USART_Transmit(':');
		USART_Transmit(' ');
		USART_Transmit('D');
		USART_Transmit('i');
		USART_Transmit('b');
		USART_Transmit('u');
		USART_Transmit('j');
		USART_Transmit('o');
		USART_Transmit(' ');
		USART_Transmit('2');
		USART_Transmit('\n');
		break;
		case 2:
		USART_Transmit('3');
		USART_Transmit(':');
		USART_Transmit(' ');
		USART_Transmit('D');
		USART_Transmit('i');
		USART_Transmit('b');
		USART_Transmit('u');
		USART_Transmit('j');
		USART_Transmit('o');
		USART_Transmit(' ');
		USART_Transmit('3');
		USART_Transmit('\n');
		break;
		case 3:
		USART_Transmit('4');
		USART_Transmit(':');
		USART_Transmit(' ');
		USART_Transmit('D');
		USART_Transmit('i');
		USART_Transmit('b');
		USART_Transmit('u');
		USART_Transmit('j');
		USART_Transmit('o');
		USART_Transmit(' ');
		USART_Transmit('4');
		USART_Transmit('\n');
		break;
		case 4:
		USART_Transmit('4');
		USART_Transmit(':');
		USART_Transmit(' ');
		USART_Transmit('M');
		USART_Transmit('o');
		USART_Transmit('d');
		USART_Transmit('o');
		USART_Transmit(' ');
		USART_Transmit('l');
		USART_Transmit('i');
		USART_Transmit('b');
		USART_Transmit('r');
		USART_Transmit('e');
		USART_Transmit('\n');
		break;

	}
	
	/*//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	_delay_ms(30);
	if (Aux1 == 1)
	{
		Aux1 = 0;
	}
	else
	{
		Aux1 = 1;
	}
	_delay_ms(30);
	*///>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}															//>interupciones<

																							// Rutina de servicio de interrupción para boton2
ISR(INT1_vect)
{															//>interupciones<
	
	_delay_ms(1000);
	switch (selectedOption)
	{
		case 0:
		state = ESTADO_OPCION_1;
		break;
		case 1:
		state = ESTADO_OPCION_2;
		break;
		case 2:
		state = ESTADO_OPCION_3;
		break;
		case 3:
		state = ESTADO_OPCION_4;
		break;
		case 4:
		state = MODO_LIBRE;
		break;
	}
	
	/*//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	_delay_ms(30);
	if (Aux2 == 1)
	{
		Aux2 = 0;
	}
	else
	{
		Aux2 = 1;
	}
	_delay_ms(30);
	*///>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
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
		{0, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 1, 0},
		{1, 0, 1, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 1}
						>tabla 1-2<
		{1, 0, 0, 0}
		{1, 0, 1, 0},
		{0, 0, 1, 0},
		{0, 1, 1, 0},
		{0, 1, 0, 0},
		{0, 1, 0, 1},
		{0, 0, 0, 1},
		{1, 0, 0, 1}
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
						>tabla 3<
		{0, 1, 0, 1},
		{0, 0, 0, 1},
		{0, 1, 1, 0},
		{0, 1, 0, 0},
		{1, 0, 1, 0},
		{0, 0, 1, 0},
		{1, 0, 0, 1},
		{0, 1, 0, 0}
						>tabla 3<
		{1, 0, 1, 0},
		{1, 0, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 1, 0},
		{0, 1, 0, 1},
		{0, 1, 0, 0},
		{1, 0, 0, 1},
		{0, 0, 0, 1}
		
				{1, 0, 0, 0},
				{1, 0, 1, 0},
				{0, 0, 1, 0},
				{0, 1, 1, 0},
				{0, 1, 0, 0},
				{0, 1, 0, 1},
				{0, 0, 0, 1},
				{1, 0, 0, 1}
-------------------------------------
*/
/*
tx 1 salida			PD1
13 salida			PB5
a0 salida			PC0
a1 salida			PC1
a2 entrada y1 axis	PC2
a3 entrada y2 axis	PC3
a4 entrada x1 axis	PC4
a5 entrada x2 axis	PC5
DDRD &= ~(1 << PIN_ENTRADA);



// Definir pines de interés
#define PIN_SALIDA PD2
#define PIN_ENTRADA PD3

// Configurar PIN_SALIDA como salida
DDRD |= (1 << PIN_SALIDA);

// Configurar PIN_ENTRADA como entrada (sin resistencia de pull-up)
DDRD &= ~(1 << PIN_ENTRADA);

// O, si se desea usar resistencia de pull-up en PIN_ENTRADA
// DDRD &= ~(1 << PIN_ENTRADA); // Configurar como entrada
// PORTD |= (1 << PIN_ENTRADA); // Activar resistencia de pull-up

*/