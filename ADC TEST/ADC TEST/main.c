#include <avr/io.h>

#define Umbral 630 //ejemplo lo tenemos que variar
#define Pin 0 // pin


void ADC_init()
{

	ADMUX = (1<<REFS0) | Pin;
	ADCSRA = (1<<ADEN)|(7<<ADPS0);
}

uint16_t ADC_read()
{

	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));

	return (ADC);
}

int main(void)
{
	uint16_t adc_result;

	ADC_init();

	DDRD |= (1<<PD1);

	while(1)
	{
		adc_result = ADC_read();

		if(adc_result > Umbral)
		PORTD |= (1<<PD1);
		else
		PORTD &= ~(1<<PD1);
	}

	return 0;
}