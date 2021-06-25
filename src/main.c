#include "stm32f0xx.h"
#include "stm32f0xx_it.h"
#include "main.h"

volatile uint8_t dim_buf[3] = {0};					// Actual Value to Reach

static void EXTI0_Config(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
  /* Enable GPIOA clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  /* Configure PA0 pin as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// ================= test ================ //
	/* Configure PA4 pin as input pull up */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	// ======================================= //
	
	  /* Configure PA1, PA2 and PA3 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	  	// output
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		// pushpull mode
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// max
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	// output should not have pull up/down
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  /* Connect EXTI0 Line to PA0 pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

  /* Configure EXTI0 line */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI0 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


static void TIM_Config(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
  /* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;     						// ARR
  TIM_TimeBaseStructure.TIM_Prescaler = AC_DIM_PRESCALER;		// PSC
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* Output Compare Timing Mode configuration: Channel1 */
	TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Active;
  TIM_OCInitStructure.TIM_Pulse = 0;												// Output compare value (CCR1)
  TIM_OC1Init(TIM3, &TIM_OCInitStructure);

  /* Output Compare Timing Mode configuration: Channel2 */
  TIM_OCInitStructure.TIM_Pulse = 0;												// Output compare value (CCR2)
  TIM_OC2Init(TIM3, &TIM_OCInitStructure);

  /* Output Compare Timing Mode configuration: Channel3 */
  TIM_OCInitStructure.TIM_Pulse = 0;												// Output compare value (CCR3)
  TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	
	/* Enable the TIM3 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	/* TIM Interrupts enable */
  TIM_ITConfig(TIM3, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3, ENABLE);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}

void UART_getAndSetDim(void)
{
	uint8_t buf[3] = {0};
	
	// TODO: Populate buff from uart
	
	if(buf[0] == 0xA0)	// header
	{
		switch (buf[1])
		{
			case 0: dim_buf[0] = buf[2]; break;	// Light 1
			case 1: dim_buf[1] = buf[2]; break;	// Light 2
			case 2: dim_buf[2] = buf[2]; break;	// Light 3
			default: break;
		}
	}
}
void CheckButton(void)
{
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == Bit_RESET)
	{
		int i;
		uint8_t dimmmer_val  = dim_buf[0] + 20;
		dimmmer_val = dimmmer_val % 120;
		dim_buf[0] = dimmmer_val;
		for(i = 0; i<300000; i++);	// delay
	}
}

int main (void)
{
	EXTI0_Config();
	TIM_Config();
	
	// Test values
	/*dim_buf[0] = 50;
	dim_buf[1] = 100;
	dim_buf[2] = 75;*/
	
	while(1)
	{
		UART_getAndSetDim();
		CheckButton();
	}
}
