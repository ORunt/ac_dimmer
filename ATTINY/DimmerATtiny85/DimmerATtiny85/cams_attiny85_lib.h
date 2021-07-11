/*
 * cams_attiny85_lib.h
 *
 * Created: 2021/07/06 13:44:00
 *  Author: Cameron
 */ 


#ifndef CAMS_ATTINY85_LIB_H_
#define CAMS_ATTINY85_LIB_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define NULL ((void*)0)
typedef char bool;
#define true	1
#define false	0

#define SYS_CLK	8000000UL	// 8MHz
#define F_CPU	1000000UL	// 1MHz
#define PRESCALER	64

// ============== Clock ==============
typedef enum{
	CLK_PSC_1 = 0,
	CLK_PSC_2,
	CLK_PSC_4,
	CLK_PSC_8,
	CLK_PSC_16,
	CLK_PSC_32,
	CLK_PSC_64,
	CLK_PSC_128,
	CLK_PSC_256,
}CLK_PSC_e;

// ============== Timer ==============
typedef enum{
	TIM0_A = 0,
	TIM0_B,
	TIM1_A,
	TIM1_B,
}TIMx_e;

typedef enum{
	TIM0_STOP = 0,
	TIM0_PSC_1,
	TIM0_PSC_8,
	TIM0_PSC_64,
	TIM0_PSC_256,
	TIM0_PSC_1024,
}TIM0_PSC_e;

typedef enum{
	TIM1_STOP = 0,
	TIM1_PSC_1,
	TIM1_PSC_2,
	TIM1_PSC_4,
	TIM1_PSC_8,
	TIM1_PSC_16,
	TIM1_PSC_32,
	TIM1_PSC_64,
	TIM1_PSC_128,
	TIM1_PSC_256,
	TIM1_PSC_512,
	TIM1_PSC_1024,
	TIM1_PSC_2048,
	TIM1_PSC_4096,
	TIM1_PSC_8192,
	TIM1_PSC_16384,
}TIM1_PSC_e;

// TIM0 TCCR
#define WGM_NORMAL	0x00
#define WGM_PWM		0x01
#define WGM_CTC		0x02

typedef void (*InterruptFunction)(uint8_t num);

void setupSystemClock(CLK_PSC_e prescaler);
uint32_t getSystemClockHz(void);
void InitialiseTimer(TIMx_e timer, InterruptFunction attach_interrupt);
void SetTimerCompare(TIMx_e timer, uint8_t compare_value);
void ResetAllCounters(void);
void initialiseExternalInterrupt(uint8_t pin, InterruptFunction attach_interrupt);
void enableGlobalInterrupts(bool enable);
void setPinOutput(uint8_t pin);
void setPin(uint8_t pin);
void resetPin(uint8_t pin);


#endif /* CAMS_ATTINY85_LIB_H_ */