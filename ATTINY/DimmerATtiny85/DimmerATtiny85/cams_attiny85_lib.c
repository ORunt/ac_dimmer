/*
 * cams_attiny85_lib.c
 *
 * Created: 2021/07/06 13:43:20
 *  Author: Cameron
 */ 

#include "cams_attiny85_lib.h"

// ============== Clock ==============

void setupSystemClock(CLK_PSC_e prescaler)
{
	if((CLKPR & 0x0F) != prescaler)
	{
		CLKPR = 0x80;
		CLKPR |= prescaler;
	}
}

uint32_t getSystemClockHz(void)
{
	uint32_t system_prescaler = _BV(CLKPR & 0x0F);
	return SYS_CLK / system_prescaler;
}

// ============== Timer ==============

InterruptFunction INT_FUNC_tim0_OCA = NULL;
InterruptFunction INT_FUNC_tim0_OCB = NULL;
InterruptFunction INT_FUNC_tim1_OCA = NULL;
InterruptFunction INT_FUNC_tim1_OCB = NULL;

void InitialiseTimer(TIMx_e timer, InterruptFunction attach_interrupt)
{
	// Unique things
	switch(timer)
	{
		case TIM0_A:
			INT_FUNC_tim0_OCA = attach_interrupt;
			// enable compare interrupt
			TIMSK |= _BV(OCIE0A);
			break;
		case TIM0_B:
			INT_FUNC_tim0_OCB = attach_interrupt;
			// enable compare interrupt
			TIMSK |= _BV(OCIE0B);
			break;
		case TIM1_A:
			INT_FUNC_tim1_OCA = attach_interrupt;
			// enable compare interrupt
			TIMSK |= _BV(OCIE1A);
			break;
		case TIM1_B:
			INT_FUNC_tim1_OCB = attach_interrupt;
			// enable compare interrupt
			TIMSK |= _BV(OCIE1B);
			break;
		default:
			break;
	}
	
	// Common things
	switch(timer)
	{
		case TIM0_A:
		case TIM0_B:
			// set prescaler and start timer
			TCCR0B |= TIM0_PSC_64;
			// initialize counter
			TCNT0 = 0;
			break;
		case TIM1_A:
		case TIM1_B:
			// set prescaler and start timer
			TCCR1 |= TIM1_PSC_64;
			// initialize counter
			TCNT1 = 0;
			break;
		default:
			break;
	}
}

void SetTimerCompare(TIMx_e timer, uint8_t compare_value)
{
	switch(timer)
	{
		case TIM0_A: OCR0A = compare_value; break;
		case TIM0_B: OCR0B = compare_value; break;
		case TIM1_A: OCR1A = compare_value; break;
		case TIM1_B: OCR1B = compare_value; break;
		default: break;
	}
}

void ResetAllCounters(void)
{
	TCNT0 = 0;
	TCNT1 = 0;
}

//TIMER 0 COMPARE A match
ISR (TIMER0_COMPA_vect){if(INT_FUNC_tim0_OCA != NULL){INT_FUNC_tim0_OCA(0);}}

//TIMER 0 COMPARE B match
ISR (TIMER0_COMPB_vect){if(INT_FUNC_tim0_OCB != NULL){INT_FUNC_tim0_OCB(1);}}

//TIMER 1 COMPARE A match
ISR (TIMER1_COMPA_vect){if(INT_FUNC_tim1_OCA != NULL){INT_FUNC_tim1_OCA(2);}}

//TIMER 1 COMPARE B match
ISR (TIMER1_COMPB_vect){if(INT_FUNC_tim1_OCB != NULL){INT_FUNC_tim1_OCB(3);}}


// ============== Interrupts ==============

#define IO_PINS		5

InterruptFunction INT_FUNC_ext_pin = NULL;

void enableGlobalInterrupts(bool enable)
{
	if(enable){
		sei();
	}else{
		cli();
	}
}

void initialiseExternalInterrupt(uint8_t pin, InterruptFunction attach_interrupt)
{
	INT_FUNC_ext_pin = attach_interrupt;
	PCMSK = _BV(pin);
	GIMSK = _BV(PCIE);
}

ISR(PCINT0_vect)
{
	if(INT_FUNC_ext_pin != NULL)
	{
		//int i;
		uint8_t port_b = PINB;
		uint8_t pcint = PCMSK;

		/*for(i = 0; i < IO_PINS; i++)
		{
			if((port_b & _BV(i)) && (pcint & _BV(i)))
			{
				INT_FUNC_ext_pin(i);
			}
		}*/
		if((port_b & _BV(3)) && (pcint & _BV(3)))
		{
			INT_FUNC_ext_pin(3);
		}
	}
}

// ============== GPIO ==============

void setPinOutput(uint8_t pin)
{
	DDRB |= _BV(pin);
}

void setPin(uint8_t pin)
{
	PORTB |= _BV(pin);
}

void resetPin(uint8_t pin)
{
	PORTB &= ~_BV(pin);
}

// ============== I2C ==============

String received;
//PB2 - SCL
//PB0 - SDA
short int flag;
short int i=0;

void initializeI2C()
{
	USICR=(1<<USIWM1)|(1<<USICS1)|(1<<USICLK);  // TWI mode
	DDRB=0x00;									// SDA & SCL direction as input
	PORTB=0x00;									// SDA & SCL default state
	USISR=0x00;									// Counter value
}


void i2c_ack()
{
	DDRB  |= _BV(PB0);		// Set the direction to output
	USISR |= (1<<USIOIF);	// clear overflow flags
	PORTB |= _BV(PB0);
	while((USISR&0x01)==0); // Wait until counter goes 1
	PORTB &= ~_BV(PB0);		// Ack bit end
	USISR &= 0xF0;			// Clear counter bits
}


void i2c_address()
{
	char usi_data;
	while(USISIF==0);		// Wait till address bit is received
	{
		usi_data=USIDR;
		if(usi_data==0x00)  // Verifying the address
		{
			i2c_ack();		// Send acknowledge bit
			flag=1;
		}
	}
}


void receive_data()
{
	if((flag==1)&&(USIOIF==1))	// Checked address and counter overflow bits
	{
		received[i]=USIDR;		// Received data stored in string
		i++;
		USISR|=(1<<USIOIF);		// Clear overflow flag
	}
}