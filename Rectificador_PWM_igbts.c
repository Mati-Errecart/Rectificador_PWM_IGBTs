/*
 * Rectificador_PWM_igbts.c
 *
 * Created: 18/1/2021 9:30:39 p. m.
 *  Author: matias gonzalo
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000ul
#include <util/delay.h>

long ValorPote;
unsigned char Resto;
short SalidaPWM;

int main(void)
{
	DDRD |=(1<<DDD5); // PD5 como salida PWM (OC0B)
	DDRB |=(1<<DDB5)|(1<<DDB4);	 // Conf como salida PB5(13) PB4(12)
	PORTD |= (1<<PORTD2)|(1<<PORTD3); // Configuro Restistencias pull up
	
	TCCR0A= (1<<COM0B1)|(0<<COM0B0)|(0<<WGM01)|(1<<WGM00);	 // PWM fase correcta,
	TCCR0B= (0<<CS02)|(1<<CS01)|(0<<CS00)|(1<<WGM02);	 // Selecciono prescaler en 8
	
	
	OCR0A= 199; // Define la frecuencia pwm 5khz
	OCR0B = 100; // Define el ancho de pulso
	
	EIMSK= (1<<INT1)|(1<<INT0);
	EICRA= (1<<ISC11)|(0<<ISC10)|(1<<ISC01)|(1<<ISC00); // Conf INT1 flanco descendente, INT0 flanco ascendente
	
	
	TCCR1A=0;
	TCCR1B=4; // Selector de reloj CKl/256
	TCNT1=30360; //3036 1s
	TIMSK1=(1<<TOIE1);  // Habilitación de interrupción por desbordamiento
	
	UCSR0A=0;
	UBRR0=103;    // Velociadad de transmicion en 9600
	UCSR0C=0B000000110;  // Tamaño de caracter 8 bit
	UCSR0B=(1<<RXEN0) | (1<<TXEN0) | (0<<RXCIE0);  // habilito transceptor, receptor, interrupcion recepcion completa
	
	ADMUX= (1<<REFS0);					// Tension de referencia con capacitor externo
	ADCSRA=(1<<ADEN)|(0<<ADSC)|(1<<ADPS1)|(1<<ADPS0);	// Habilito el ADC, Configuro prescaler en 8
	
	
	sei();
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}
ISR (INT1_vect) // PD3(pin3)
{
	
	PORTB &= ! (1<<PORTB4); // En 0 el PINB4
	_delay_us(100);
	PORTB |= (1<<PORTB5); // En alto PB5(13) Semiciclo negativo

}

ISR (INT0_vect) // PD2(pin2)
{
	PORTB &= !(1<<PORTB5); // Apago el PB5
	_delay_us(100);
	PORTB |= (1<<PORTB4); // En alto PB4(12) Semiciclo positivo
	
}

// RUTINA DE TRATAMIENTO DE INTERRUPCION DEL CAD

ISR(TIMER1_OVF_vect)
{
	
	ADCSRA|=(1<<ADSC);     //Iniciamos conversion
	while(ADCSRA & (1<<ADSC));
	ValorPote=ADC;
	ValorPote= (long)ValorPote*100/1023; //Guardamos el valor ADC en una variable
	UDR0=(ValorPote/100)+48;
	while(!(UCSR0A & (1<<UDRE0)));	//Espera a que se envíe el dato
	Resto=(ValorPote%100);
	UDR0=(Resto/10)+48;
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0=((Resto%10)+48);
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0=10;
	while(!(UCSR0A & (1<<UDRE0)));
	TCNT1= 30360;		//1 seg para realizar la conversion
	OCR0B= ValorPote*199/100;	 	 // PD5 salida PWM
}