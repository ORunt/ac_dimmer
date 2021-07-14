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
			TCCR0B |= TIM0_PSC_1024;
			// initialize counter
			TCNT0 = 0;
			break;
		case TIM1_A:
		case TIM1_B:
			// set prescaler and start timer
			TCCR1 |= TIM1_PSC_1024;
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
		int i;
		uint8_t port_b = PINB;
		uint8_t pcint = PCMSK;

		for(i = 0; i < IO_PINS; i++)
		{
			if((port_b & _BV(i)) && (pcint & _BV(i)))
			{
				INT_FUNC_ext_pin(i);
			}
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

#define I2C_SCL			PB2
#define I2C_SDA			PB0
#define I2C_ADDRESS		0x6A

#define GET_USISIF		((USISR & _BV(USISIF)) == _BV(USISIF))
#define GET_USIOIF		((USISR & _BV(USIOIF)) == _BV(USIOIF))
#define GET_USIPF		((USISR & _BV(USIPF))  == _BV(USIPF))

#define I2C_SET_SDA_OUTPUT()	{ DDRB |=  _BV(I2C_SDA); }
#define I2C_SET_SDA_INPUT() 	{ DDRB &= ~_BV(I2C_SDA); }
#define I2C_SET_BOTH_INPUT() 	{ DDRB &= ~(_BV(I2C_SDA) | _BV(I2C_SCL)); }

#define I2C_ACK_USISR			0x7E	// Counts one clock (ACK)
#define I2C_BYTE_USISR			0x70	// Counts 8 clocks (BYTE)
#define I2C_CLR_START_USISR		0xF0	// Clears START flag
#define I2C_SET_START_USISR		0x70	// Sets START flag


void plotValue(uint8_t val)
{
	int i,j;
	for(i = 0; i < 8; i++){
		if(val & _BV(7-i)) { PORTB |= _BV(PB3); }
		else { PORTB &= ~_BV(PB3); }
		for (j=0; j<10; j++){__asm__("nop");}
	}
	PORTB &= ~_BV(PB3);
}


void i2c_init(void)
{
	USICR = _BV(USIWM1)|_BV(USICS1);	// I2C Slave mode
	I2C_SET_BOTH_INPUT() 				// SDA & SCL direction as input
	USISR = I2C_CLR_START_USISR;		// Counter value (counts SCL pulses)
}


static void i2c_ack(void)
{
	USIDR = 0;
	I2C_SET_SDA_OUTPUT()			// Set the direction to output
	PORTB = PINB & ~_BV(I2C_SDA);	// Pull the SDA line low to send ACK
	USISR = I2C_ACK_USISR;			// Set counter to overflow in 1 clock
	while(GET_USIOIF==0);			// Wait for overflow
	I2C_SET_SDA_INPUT()				// Set the direction to input
	USISR = I2C_BYTE_USISR;			// Reset counter to 0
}


uint8_t i2c_receive_data(uint8_t * buf, uint8_t size)
{
	uint8_t usi_data;
	uint8_t len = 0;
	
	// ================= Start sequence =================
	
	while(GET_USISIF==0);										// Wait for start bit (address bit is received)
	while((PINB & _BV(I2C_SCL)) && !(PINB & _BV(I2C_SDA)));		// Wait for SCL to go high and SDA to go low
	USISR = I2C_CLR_START_USISR;							    // Acknowledge Start bit and reset SCL counter
	
	// ================= Verify I2C Address =================
	
	while(GET_USIOIF==0);			// Wait for counter overflow
	usi_data = USIDR>>1;			// Save Address
	
	if((usi_data) == I2C_ADDRESS)	// Verify I2C address
	{
		i2c_ack();					// Send acknowledge bit
	}
	else
	{
		USISR = I2C_SET_START_USISR;// Clear any bits and set counter back to 0 for next session
		return 0;					// This message was not for us
	}
	
	// ===================== Receive Data =====================
	
	while(len < size)				// Unfortunately waiting for a stop bit never happens, 
	{								// so we hard code check the length here instead
		while(GET_USIOIF==0);		// Wait for counter overflow
		buf[len++] = USIDR;			// Received data stored in buffer
		i2c_ack();					// Send acknowledge bit
	}
	
	return len;
}