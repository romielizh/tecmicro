/*
 * main.c
 *
 * Created: 11/7/2023 10:00:19 AM
 *  Author: romin
 */ 

#include <xc.h>
///////////////////////////////////////////////////LIBRERÍA UART//////////////////////////////////////////
#include <avr/io.h>
#include <util/setbaud.h>

#define desplazamiento(bit) (1 << (bit))
#define bit_esta_seteado(sfr, bit) (_SFR_BYTE(sfr) & desplazamiento(bit))
#define bit_esta_apagado(sfr, bit) (!(_SFR_BYTE(sfr) & desplazamiento(bit)))
#define bucle_hasta_que_bit_es_seteado(sfr, bit) do { } while (bit_esta_apagado(sfr, bit))
#define bucle_hasta_que_bit_es_apagado(sfr, bit) do { } while (bit_esta_seteado(sfr, bit))

/*_SFR_BYTE(sfr) es un macro definido en avr/io.h, lo que haces es devolver el Byte (8 bits) de un puerto en específico, es decir, devuelve el valor en bits actual en el que se encuentra el puerto, ejemplo: _SFR_BYTE(PORTB) puede devolver algo como 0b11000101. Lo que recibe el macro (sfr) significa "special function register" por lo que se puede utilizar también con registros como UCSR0A*/


void inicializar_USART(void) { /* requiere un baud rate */
	UBRR0H = UBRRH_VALUE; /* definido dentro de util/setbaud.h */
	UBRR0L = UBRRL_VALUE; /* lo mismo que lo anterior*/
	#if USE_2X
	UCSR0A |= (1 << U2X0);
	#else
	UCSR0A &= ~(1 << U2X0);
	#endif
	/* Habilita la USART como transmisor/receptor */
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); /* 8 bits de datos, 1 bit de paro*/
}
void transmitir_Byte(uint8_t datos) {
	/* Espera por un buffer de transmisión vacío*/
	bucle_hasta_que_bit_es_seteado(UCSR0A, UDRE0); /*bucle hasta que bit UDRE0 es seteado en UCSR0A*/
	UDR0 = datos; /* envía los datos*/
}

uint8_t recibir_Byte(void) {
	bucle_hasta_que_bit_es_seteado(UCSR0A, RXC0); /* espera por llegada de datos (datos entrantes)*/
	return UDR0; /*devuelve el valor actual del registro UDR0 */
}
// Ejemplo de función util para imprimir mensajes / datos en USART
void imprimir_Cadena(const char cadena[]) {
	uint8_t i = 0; //uint8_t = tipo de dato unsigned char
	while(cadena[i]) {
		transmitir_Byte(cadena[i]);
		i++;
	}
}

void imprimir_Byte(uint8_t byte){
	/* Convierte un byte a formato texto cadena / string decimal y lo envía */
	transmitir_Byte('0'+ (byte/100)); /* centenas*/
	transmitir_Byte('0'+ ((byte/10) % 10)); /* decenas*/
	transmitir_Byte('0'+ (byte % 10)); /* unidades*/
}

void imprimir_binario_Byte(uint8_t byte){
	/* Imprime un byte como una serie o secuencia de 1 y 0*/
	uint8_t bit;
	for(bit = 7; bit < 255; bit--){
		if(bit_esta_seteado(byte, bit)){
			transmitir_Byte('1');
			}else{
			transmitir_Byte('0');
		}
	}
}
/////////////////////////////////////FIN LIBRERÍA UART//////////////////////////////////////////////////////////////////

int main(void)
{
    while(1)
    {
        
    }
}