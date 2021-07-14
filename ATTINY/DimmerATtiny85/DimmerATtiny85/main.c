/*
 * DimmerATtiny85.c
 *
 * Created: 2021/07/05 19:50:03
 * Author : Cameron
 */ 

#include "cams_attiny85_lib.h"

#define LIGHTS				1

#define AC_DIM_MIN_PERCENT	20
#define AC_DIM_MAX_PERCENT	95

#define LIGHT_PIN_0			4
#define LIGHT_PIN_1			1
#define LIGHT_PIN_2			2
#define ZERO_CROSS_PIN		3

#define I2C_PACKET_SIZE		2	// excluding the address

typedef struct{
	uint8_t zero_cross;		// Set when a zero cross happens
	uint8_t dim_trans_buf;	// The current dim value
	uint8_t dim_buf;		// The next dim value
}light_store_t;

volatile light_store_t light_store[LIGHTS] = {0};

// Dim value between 0 (off) and 100 (max)
uint8_t Calc_Dim_CCR(uint32_t dim)
{
	dim = (dim < 100) ? dim : 100;
	// 10ms is max brightness
	//uint32_t t_cnt_ns = (AC_DIM_PRESCALER + 1) * 1000000000 / SystemCoreClock; // In ns
	//return dim * 100000 / t_cnt_ns;

	return (uint8_t)(((100 - dim) * F_CPU) / (PRESCALER * 10000UL));
}

uint8_t map_pin(uint8_t pin)
{
	switch(pin)
	{
		case 0: return LIGHT_PIN_0;
		case 1: return LIGHT_PIN_1;
		case 2: return LIGHT_PIN_2;
		default: return LIGHT_PIN_0;
	}
}

void isr_light(uint8_t num)
{
	if(light_store[num].zero_cross)
	{
		light_store[num].zero_cross = 0;

		if(light_store[num].dim_trans_buf > AC_DIM_MIN_PERCENT)
		{
			setPin(map_pin(num));
		}
	}
	
	if(light_store[num].dim_trans_buf != light_store[num].dim_buf)
	{
		light_store[num].dim_trans_buf = light_store[num].dim_buf;
		SetTimerCompare((TIMx_e)num, Calc_Dim_CCR(light_store[num].dim_trans_buf));
	}
}

void isr_zeroCross(uint8_t num)
{
	int i;
	
	// Make sure all zero cross's have been cleared
	for (i = 0; i < LIGHTS; i++){
		if(light_store[i].zero_cross){
			return;
		}
	}
	
	for (i = 0; i < LIGHTS; i++)
	{
		// Zero Cross just happened
		light_store[i].zero_cross = 1;

		// Turn TRIACs off if it shouldn't stay on
		if(light_store[i].dim_trans_buf < AC_DIM_MAX_PERCENT)
		{
			resetPin(map_pin(i));
		}
	}
	
	// Start the counter from 0 again
	ResetAllCounters();
}

void timer_init(void)
{
	int i;
	
	for(i = 0; i < LIGHTS; i++){
		InitialiseTimer((TIMx_e)i, isr_light);
	}
}

void gpio_init(void)
{
	int i;
	
	for(i = 0; i < LIGHTS; i++){
		setPinOutput(map_pin(i));
		resetPin(map_pin(i));
	}
}

void exti_init(void)
{
	initialiseExternalInterrupt(ZERO_CROSS_PIN, isr_zeroCross);
}


int main(void)
{
	uint8_t buf[I2C_PACKET_SIZE] = {0};
	uint8_t len = 0;
		
	//setupSystemClock(CLK_PSC_1);	// If DIV8 fuse is not removed, then add this line
	//gpio_init();
	//timer_init();
	//exti_init();
	setPinOutput(PB3);
	resetPin(PB3);
	i2c_init();
	enableGlobalInterrupts(true);
	
    while(1)
    {
	    len = i2c_receive_data(&buf[0], I2C_PACKET_SIZE);
		
		if(len == I2C_PACKET_SIZE)
		{
			light_store[buf[0]].dim_buf = buf[1];
		}
		
    }
}
