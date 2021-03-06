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
#define true    1
#define false   0

#define SYS_CLK     20000000UL    // 20MHz (overclocked)
#define F_CPU       20000000UL    // 20MHz (overclocked)
#define PRESCALER   1024

// ============== Clock ==============

#define overclock() OSCCAL = 163

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

// ============= Watchdog ==============
#define WDT_16ms    0x00
#define WDT_32ms    0x01
#define WDT_64ms    0x02
#define WDT_125ms   0x03
#define WDT_250ms   0x04
#define WDT_500ms   0x05
#define WDT_1s      0x06
#define WDT_2s      0x07
#define WDT_4s      0x20
#define WDT_8s      0x21


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
#define WGM_NORMAL  0x00
#define WGM_PWM     0x01
#define WGM_CTC     0x02

typedef void (*InterruptFunction)(uint8_t num);

void setupSystemClock(CLK_PSC_e prescaler);
uint32_t getSystemClockHz(void);
void calibrateClockTest(void);
void watchdogSetup(void);
void feedWatchdog(void);
void InitialiseTimer(TIMx_e timer, InterruptFunction attach_interrupt);
void SetTimerCompare(TIMx_e timer, uint8_t compare_value);
void ResetAllCounters(void);
void initialiseExternalInterrupt(uint8_t pin, InterruptFunction attach_interrupt);
void enableGlobalInterrupts(bool enable);
void setPinOutput(uint8_t pin);
void setPin(uint8_t pin);
void resetPin(uint8_t pin);
void pulsePin(uint8_t pin);
void i2c_init(void);
uint8_t i2c_receive_data(uint8_t * buf, uint8_t size);
void plotValue(uint8_t val);

#endif /* CAMS_ATTINY85_LIB_H_ */