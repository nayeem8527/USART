/*
 * Demonstration on how to redirect stdio to UART. 
 */
#include <stdio.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "uartLibrary.h"

void Adcsetup()
{
  cli();
  ADMUX = 0b00000000; // AREF,Right Adjusted,Analog channel Selectio Bits: 0000 for ADC0
  ADCSRA = 0b10101111; // ADC Enable, ADC Start Conversion - off, Auto Trigger - on , ADC Interrupt Flag - off,ADC interrupt enabel -on
  ADCSRB = 0b00000011;// Timer 0 overflow interrupt enabled
  DIDR0 = 0b11111100; // disable unnecessary pins
  sei();
}

void Timer0Setup()
{
  cli();
  TCNT0 = 0x00;
  OCR0A = 255;
  TCCR0A = 0b00000010; // last two bits Timer Overflow 
  TCCR0B = 0b00000101; // input capture noise canceller deactivated ; Prescalar 101 - 1024; Normal Mode
  TIMSK0 = 0b00000010; // using Timer1 compare match 100
  sei();
}

void sleepsetup()
{
  cli();
  SMCR = 0b00000001; // 000 for Idle Mode
  MCUCR = 0b01100000;
  MCUCR = 0b01000000;
  PRR = 0b11000100; // TWI,Timer2,Timer0,Reserved,SPI,USART0,ADC
  sei();
}

void Timer1Setup()
{
	cli ();
	//COM7-4 = 0(OC1AorB disconnected),WGM11:0 = 0(CTC mode) 
	TCCR1A = 0b00000000; 
	//WGM13-2 = 01 (CTC for COMPA),CS12-0 = 101 (Prescaler = 1024) 
	TCCR1B = 0b00001101;
	TCNT1 = 0x00;
	OCR1A = 31251;				// 2 Sec
	TIMSK1 = 0b00000010;			// Set up Interrrupt for OCR1A
	sei ();
}
uint16_t ldrValue,ntcValue;

int main(void) {    
	DDRB |= 0x20;
	PORTB |= 0x20;
	DDRD |= 0b11000000;
	PORTD |= 0b11000000;
	DDRC &= 0b11111100;
	Adcsetup();
	Timer0Setup();
	Timer1Setup();
	sleepsetup(); 	
	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;
	while(1) 
	{
		sei();
		sleep_cpu();	        
	}
	return 0;
}
ISR(TIMER0_COMPA_vect)
{ 
  cli();
  PORTB &= 0b11011111;
}
// ADC - 0 :LDR   ;  ADC - 1 :Thermistor 
int x = 0;
ISR(ADC_vect)
{
  	cli ();
        //PORTB^=0x20;
        SMCR &= 0b11111110;
	uint16_t valueADC, valL, valH;
	valL = ADCL;
	valH = ADCH;
	valueADC = (valH << 8) + valL;
	if (ADMUX == 0x00)
	{      
		ldrValue = valueADC;
                ADMUX = 0x01;
		if (valueADC <= 0x000A)
                        {  
                            PORTD |= 0b01000000;      
                        }
		else
			{
                           PORTD &= 0b10111111; 
                            // led6 on
                        }    
        }
	else
	{
		ntcValue = valueADC;
		ADMUX = 0x00;
		if (valueADC >= 0x07E)
			PORTD |= 0b10000000;
		else
			PORTD &= 0b01111111;
	}         
        TCNT0 = 0x00;
        sei();
} 
ISR (USART_RX_vect)
{
	cli ();
	char input = getchar ();
	int val1=PORTD &0b01000000;
	int val2 = PORTD & 0b10000000;
	if (input == 'a' || input == 'A')	// Show stats if "a" pressed
	{
		printf ("\n\n\n");
		printf ("LED Status via Button Press\n");
		if(val1)
			printf("LDR----- On\n");
		else
			printf("LDR----- Off\n");
		if(val2)
			printf("Thermistor----- On\n");
		else
			printf("Thermistor----- Off\n");
		printf ("\n");
	}
	else
	{
		printf ("\n\n\n");
		printf ("Press A\n");
		printf ("\n\n\n");
	}

	sei ();
}
ISR (TIMER1_COMPA_vect)
{
	cli ();
	int val1=PORTD &0b01000000;
	int val2 = PORTD & 0b10000000;
	printf ("ADC Statistics\n");
	if (val1)			// Check LDR_LED Status
	{
		printf ("LDR:  Value = %d\n",ldrValue);
		printf ("LED_LDR = ON\n");
	}
	else
	{
		printf ("LDR:  Value = %d\n",ldrValue);
		printf ("LED_LDR = OFF\n");
	}
	if (val2)			// Check NTC Th_LED Status
	{
		printf ("Thermistor:  Value = %d\n",ntcValue);
		printf ("LED_Thermistor = ON\n");
	}
	else
	{
		printf ("Thermistor:  Value = %d\n",ntcValue);
		printf ("LED_Thermistor = OFF\n");
	}
	printf ("\n\n\n");
	sei ();
}


