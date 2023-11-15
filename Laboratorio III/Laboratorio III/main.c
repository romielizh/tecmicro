//?????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>									//>interrupciones<

//?????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//

#define F_CPU 16000000UL									//>UART<
#define BAUD 9600											//>UART<
#define MYUBRR F_CPU/16/BAUD-1								//>UART<
																							// Pines conectados a los botones
#define BOTON1_PIN PIND2									//>interrupciones<
#define BOTON2_PIN PIND3									//>interrupciones<
unsigned int MENU = 1;										//>interrupciones<
unsigned int LIBRE = 0;										//>interrupciones<
unsigned int izquierda = 0;									//>interrupciones<
unsigned int derecha = 0;									//>interrupciones<
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
char strings[] = "Modo Libre: Activado\n";
char stringf[] = "Modo Libre: Desactivado\n";
char string1[] = "1: Dibujo 1\n";
char string2[] = "2: Dibujo 2\n";
char string3[] = "3: Dibujo 3\n";
char string4[] = "4: Dibujo 4\n";
char stringml[] = "5: Modo Libre\n";
char stringds[] = "Dibujando...\n";
char stringdf[] = "Dibujo terminado\n";

//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//
//>Configuraciones<
void ConfigInterrupciones()
{															//>interrupciones<
	EICRA |= (1 << ISC01) | (1 << ISC11);													// Configurar boton1 y boton2 para flanco de bajada
	EIMSK |= (1 << INT0) | (1 << INT1);														// Habilitar boton1 y boton2
	sei();																					// Habilitar interrupciones globales
}															//>interrupciones<

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
		{1, 0, 1, 0},
		{0, 0, 1, 0},
		{0, 1, 1, 0},
		{0, 1, 0, 0},
		{0, 1, 0, 1},
		{0, 0, 0, 1},
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
//>Secuencia_pasos_Y<
void setStepY(int step)
{															//>motor2<
																							// Tabla de pasos
	int sequence[8][4] =																	//[filas][columnas]
	{
		{1, 0, 0, 0},
		{1, 0, 1, 0},
		{0, 0, 1, 0},
		{0, 1, 1, 0},
		{0, 1, 0, 0},
		{0, 1, 0, 1},
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
// Funciones_1/4_circulo
void circulopasosX(unsigned int PASITOS)
{
	int i = 0;																				// Variable para los pasos
	PASOS = 0;																				// Variable que cuenta los pasos hechos
	while (PASOS < PASITOS)																	// Mientras los pasos hechos sean menores a los pasos que hay que hacer
	{
		if (i==8)																			// Se completó una revolucion?
		{i=0;}																				// Entonces comenzar desde el paso 0
		setStepX(i);																		// Dar el paso numero i
		_delay_ms(150);																		// Tiempo entre pasos
		i++;																				// Paso siguiente
		PASOS++;																			// Se dio otro paso mas
	}
}

void circulopasosY(unsigned int PASITOS)
{
	int i = 0;																				// Variable para los pasos
	PASOS = 0;																				// Variable que cuenta los pasos hechos
	while (PASOS < PASITOS)																	// Mientras los pasos hechos sean menores a los pasos que hay que hacer
	{
		if (i==8)																			// Se completó una revolucion?
		{i=0;}																				// Entonces comenzar desde el paso 0
		setStepY(i);																		// Dar el paso numero i
		_delay_ms(150);																		// Tiempo entre pasos
		i++;																				// Paso siguiente
		PASOS++;																			// Se dio otro paso mas
	}
}

//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//
//>Secuencia_ejecucion_dibujos_UART<
void handle_state()
{															//>UART<
	switch (state)
	{
		case ESPERAR:
			break;
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>LINEA
		case ESTADO_OPCION_1:																// Dibujo 1 start 
			MENU = 0;
			for (int i = 0; i < 13; i++)
				{USART_Transmit(stringds[i]);}												// "Dibujando...\n"
			PORTB |= (1 << SOL_PIN);						//>solenoide< ON
			while (PASOS < 100)
			{
				for (int i = 0; i < 8; i++)													// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					//setStepX(i);
					setStepY(i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
					PASOS++;
				}
			}
			state = ESPERAR;																// Volver a MENU, seleccion de opciones
			MENU = 1;
			for (int i = 0; i < 17; i++)
				{USART_Transmit(stringdf[i]);}												// "Dibujo terminado\n"
			PORTB &= ~(1 << SOL_PIN);						//>solenoide< OFF
			break;																			// Dibujo 1 stop
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>CUADRADO
		case ESTADO_OPCION_2:																// Dibujo 2 start
			MENU = 0;
			for (int i = 0; i < 13; i++)
				{USART_Transmit(stringds[i]);}												// "Dibujando...\n"
			PORTB |= (1 << SOL_PIN);						//>solenoide< ON
			while (PASOS < 100)
			{
				for (int i = 0; i < 8; i++)													// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					//setStepX(i);
					setStepY(i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
					PASOS++;
				}
			}
			PASOS = 0;
			_delay_ms(1500);
			while (PASOS < 100)
			{
				for (int i = 0; i < 8; i++)													// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					setStepX(i);
					//setStepY(i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
					PASOS++;
				}
			}
			PASOS = 0;
			_delay_ms(1500);
			while (PASOS < 100)
			{
				for (int i = 8; i >= 0; i--)												// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					//setStepX(i);
					setStepY(i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
					PASOS++;
				}
			}
			PASOS = 0;
			_delay_ms(1500);
			while (PASOS < 100)
			{
				for (int i = 8; i >= 0; i--)												// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					//setStepX(i);
					setStepY(i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
					PASOS++;
				}
			}
			PASOS = 0;
			_delay_ms(1500);
			state = ESPERAR;																// Volver a MENU, seleccion de opciones
			MENU = 1;
			for (int i = 0; i < 17; i++)
				{USART_Transmit(stringdf[i]);}												// "Dibujo terminado\n"
			PORTB &= ~(1 << SOL_PIN);						//>solenoide< OFF
			break;
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ROMBO
		case ESTADO_OPCION_3:
			MENU = 0;
			for (int i = 0; i < 13; i++)
				{USART_Transmit(stringds[i]);}												// "Dibujando...\n"
			PORTB |= (1 << SOL_PIN);						//>solenoide< ON
			while (PASOS < 100)
			{
				for (int i = 0; i < 8; i++)													// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					setStepX(i);
					setStepY(i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
					PASOS++;
				}
			}
			PASOS = 0;
			_delay_ms(1500);
			while (PASOS < 100)
			{
				for (int i = 0; i < 8; i++)													// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					setStepX(i);
					setStepY(8-i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
					PASOS++;
				}
			}
			PASOS = 0;
			_delay_ms(1500);
			while (PASOS < 100)
			{
				for (int i = 8; i >= 0; i--)												// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					setStepX(i);
					setStepY(i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
					PASOS++;
				}
			}
			PASOS = 0;
			_delay_ms(1500);
			while (PASOS < 100)
			{
				for (int i = 8; i >= 0; i--)												// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					setStepX(i);
					setStepY(8-i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
					PASOS++;
				}
			}
			PASOS = 0;
			_delay_ms(1500);
			state = ESPERAR;																// Volver a MENU, seleccion de opciones
			MENU = 1;
			for (int i = 0; i < 17; i++)
				{USART_Transmit(stringdf[i]);}												// "Dibujo terminado\n"
			PORTB &= ~(1 << SOL_PIN);						//>solenoide< OFF
			break;
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>SINUSOIDE
		case ESTADO_OPCION_4:
			MENU = 0;
			setStepX(7);// Energizar motores (paso 0)
			setStepY(7);// Energizar motores (paso 0)
		
			circulopasosX(16);// 16 pasos eje x (comienza desde 0, al llegar a 16 se detiene, por lo que se ejecutan 16 pasos, sucede lo mismo en todos)
			circulopasosY(1);// 1 paso en eje y
		
			circulopasosX(11);// 11 pasos eje x (ultimo comentario, ya se entiende)
			circulopasosY(1);
		
			circulopasosX(8);
			circulopasosY(1);

			circulopasosX(7);
			circulopasosY(1);

			for (int i = 0; i < 3;i++){// Se repite 3 veces la secuencia de 5 pasos en eje x, 1 en eje y
				circulopasosX(5);
			circulopasosY(1);}

			for (int i = 0; i < 2;i++){
				circulopasosX(4);
				circulopasosY(1);}

			circulopasosX(3);
			circulopasosY(1);

			circulopasosX(4);
			circulopasosY(1);

			for	(int i = 0; i < 5;i++){
				circulopasosX(3);
				circulopasosY(1);}

			for (int i = 0; i < 2;i++){
				circulopasosX(2);
				circulopasosY(1);
				circulopasosX(3);
				circulopasosY(1);}

			for (int i = 0; i < 2;i++){
				circulopasosX(2);
				circulopasosY(1);}

			circulopasosX(3);
			circulopasosY(1);

			for (int i = 0; i < 8;i++){
				circulopasosX(2);
				circulopasosY(1);}

			circulopasosX(1);
			circulopasosY(1);

			for (int i = 0; i < 3;i++){
				circulopasosX(2);
				circulopasosY(1);}

			circulopasosX(1);
			circulopasosY(1);

			for (int i = 0; i < 2;i++){
				circulopasosX(2);
				circulopasosY(1);}

			for (int i = 0; i < 5;i++){
				circulopasosX(1);
				circulopasosY(1);
				circulopasosX(2);
				circulopasosY(1);}

			for (int i = 0; i < 2;i++){
				circulopasosX(1);
				circulopasosY(1);}

			circulopasosX(2);
			circulopasosY(1);

			for (int i = 0; i < 2;i++){
				circulopasosX(1);
				circulopasosY(1);}

			circulopasosX(2);
			circulopasosY(1);

			for (int i = 0; i < 3;i++){
				circulopasosX(1);
				circulopasosY(1);}

			circulopasosX(2);
			circulopasosY(1);

			for (int i = 0; i < 6;i++){
				circulopasosX(1);
				circulopasosY(1);}

			circulopasosX(2);
			circulopasosY(1);

			for (int i = 0; i < 9;i++){
				circulopasosX(1);
				circulopasosY(1);}

		// 1/8 de circulo, en adelante es el mismo codigo en espejo, invirtiendo los ejes, para completar 1/4 de circulo

			for (int i = 0; i < 9;i++){
				circulopasosY(1);
				circulopasosX(1);}

			circulopasosY(2);
			circulopasosX(1);

			for (int i = 0; i < 6;i++){
				circulopasosY(1);
				circulopasosX(1);}

			circulopasosY(2);
			circulopasosX(1);

			for (int i = 0; i < 3;i++){
				circulopasosY(1);
				circulopasosX(1);}

			circulopasosY(2);
			circulopasosX(1);

			for (int i = 0; i < 2;i++){
				circulopasosY(1);
				circulopasosX(1);}

			circulopasosY(2);
			circulopasosX(1);

			for (int i = 0; i < 2;i++){
				circulopasosY(1);
				circulopasosX(1);}

			for (int i = 0; i < 5;i++){
				circulopasosY(2);
				circulopasosX(1);
				circulopasosY(1);
				circulopasosX(1);}

			for (int i = 0; i < 2;i++){
				circulopasosY(2);
				circulopasosX(1);}

			circulopasosY(1);
			circulopasosX(1);

			for (int i = 0; i < 3;i++){
				circulopasosY(2);
				circulopasosX(1);}

			circulopasosY(1);
			circulopasosX(1);

			for (int i = 0; i < 8;i++){
				circulopasosY(2);
				circulopasosX(1);}

			circulopasosY(3);
			circulopasosX(1);

			for (int i = 0; i < 2;i++){
				circulopasosY(2);
				circulopasosX(1);}

			for (int i = 0; i < 2;i++){
				circulopasosY(3);
				circulopasosX(1);
				circulopasosY(2);
				circulopasosX(1);}

			for (int i = 0; i < 5;i++){
				circulopasosY(3);
				circulopasosX(1);}

			circulopasosY(4);
			circulopasosX(1);

			circulopasosY(3);
			circulopasosX(1);

			for (int i = 0; i < 2;i++){
				circulopasosY(4);
				circulopasosX(1);}

			for (int i = 0; i < 3;i++){
				circulopasosY(5);
				circulopasosX(1);}

			circulopasosY(7);
			circulopasosX(1);

			circulopasosY(8);
			circulopasosX(1);

			circulopasosY(11);
			circulopasosX(1);

			circulopasosY(16);
			circulopasosX(1);
		// 1/4 de circulo completo
			state = ESPERAR;
			MENU = 1;
			break;
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>MODO LIBRE
		case MODO_LIBRE:
			MENU = 0;
			LIBRE = 1;
			for (int i = 0; i < 12; i++)
				{USART_Transmit(strings[i]);}												// "Modo Libre: Activado\n"
			state = ESPERAR;
			PORTB |= (1 << SOL_PIN);						//>solenoide< ON
			break;
	}
}															//>UART<

//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//
// Codigo_principal
int main(void)
{
															//>motor1<
	DDRB |= (1 << XIN1) | (1 << XIN2) | (1 << XIN3) | (1 << XIN4);							// Configura los pines de control del puente HX como salidas
															//>motor1<
															
															//>motor2<
	DDRD |= (1 << YIN1) | (1 << YIN2) | (1 << YIN3) | (1 << YIN4);							// Configura los pines de control del puente HY como salidas
															//>motor2<

															//>interrupciones<
	DDRD &= ~((1 << BOTON1_PIN) | (1 << BOTON2_PIN));										// Configurar pines de entrada para los botones
	ConfigInterrupciones();																	// Llamada a la funcion configinterupciones
															//>interrupciones<

															//>solenoide<
	DDRB |= (1 << SOL_PIN);																	// Configura pin solenoide como salida
															//>solenoide<

															//>UART<
    USART_Init(MYUBRR);																		// Configura UART
															//>UART<

	while (1)
	{
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		if (MENU==1)
		{
			handle_state();
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		if (LIBRE==1)
		{
			while (derecha==1)
			{
				if (izquierda == 1)
				{
					MENU = 1;
					LIBRE = 0;
					derecha = 0;
					izquierda = 0;
					for (int i = 0; i < 24; i++)
						{USART_Transmit(stringf[i]);}										// "Modo Libre: Desactivado\n"
					PORTB &= ~(1 << SOL_PIN);				//>solenoide< OFF
				}
				for (int i = 0; i < 8; i++)													// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					//setStepX(i);
					setStepY(i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
				}
			}
			while (izquierda==1)
			{
				if (derecha == 1)
				{
					MENU = 1;
					LIBRE = 0;
					derecha = 0;
					izquierda = 0;
					for (int i = 0; i < 24; i++)
						{USART_Transmit(stringf[i]);}										// "Modo Libre: Desactivado\n"
					PORTB &= ~(1 << SOL_PIN);				//>solenoide< OFF
				}
				for (int i = 8; i >= 0; i--)												// Girar<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				{
					//setStepX(i);
					setStepY(i);
					_delay_ms(150);															// Ajuste de tiempo entre pasos (filas) según la velocidad deseada
				}
			}
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	}

	return 0;
}
//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//
// Rutina_interrupción_boton1
ISR(INT0_vect)
{															//>interrupciones<
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>MENU>>
	if (MENU==1)
	{
		_delay_ms(1000);
		selectedOption = (selectedOption + 1) % 5;
		switch (selectedOption)																// Seleccion de menu
		{
			case 0:
				for (int i = 0; i < 12; i++)
					{USART_Transmit(string1[i]);}											// "1: Dibujo 1\n"
				break;
			case 1:
				for (int i = 0; i < 12; i++)
					{USART_Transmit(string2[i]);}											// "2: Dibujo 2\n"
				break;
			case 2:
				for (int i = 0; i < 12; i++)
					{USART_Transmit(string3[i]);}											// "3: Dibujo 3\n"
				break;
			case 3:
				for (int i = 0; i < 12; i++)
					{USART_Transmit(string4[i]);}											// "4: Dibujo 4\n"
				break;
			case 4:
				for (int i = 0; i < 14; i++)
					{USART_Transmit(stringml[i]);}											// "5: Modo Libre\n"
				break;
		}
		_delay_ms(1000);
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<MENU>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>LIBRE>
	if (LIBRE==1)
	{
		_delay_ms(1000);
		if (derecha == 1)
		{
			derecha = 0;
		}
		else
		{
			derecha = 1;
		}
		_delay_ms(1000);
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<LIBRE>
}															//>interrupciones<

//????????????????????????????????????????????????????????????????????????????????????????//-----------------------------------------------------------------------------//
// Rutina_interrupción_boton2
ISR(INT1_vect)
{															//>interrupciones<
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>MENU>>
	if (MENU==1)
	{
		_delay_ms(1000);
		switch (selectedOption)																// Ejecucion de menu
		{
			case 0:
				state = ESTADO_OPCION_1;													// Ejecucion codigo dibujo 1
				break;
			case 1:
				state = ESTADO_OPCION_2;													// Ejecucion codigo dibujo 2
				break;
			case 2:
				state = ESTADO_OPCION_3;													// Ejecucion codigo dibujo 3
				break;
			case 3:
				state = ESTADO_OPCION_4;													// Ejecucion codigo dibujo 4
				break;
			case 4:
				state = MODO_LIBRE;															// Inicio modo libre
				break;
		}
		_delay_ms(1000);
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<MENU>>
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>LIBRE>
	if (LIBRE==1)
	{
		_delay_ms(1000);
		if (izquierda == 1)
		{
			izquierda = 0;
		}
		else
		{
			izquierda = 1;
		}
		_delay_ms(1000);
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<LIBRE>
}															//>interrupciones<




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
-------------------------------------
*/