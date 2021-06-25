/**
  ******************************************************************************
  * @file    EXTI/EXTI_Example/stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    24-July-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include "main.h"

/** @addtogroup STM32F0xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup EXTI_Example
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/
volatile uint8_t zero_cross[3] = {0};
volatile uint8_t dim_trans_buf[3] = {0};		// The incremental fade value
extern volatile uint8_t dim_buf[3];					// Actual Value to Reach

void NMI_Handler(void){}
void SVC_Handler(void){}
void PendSV_Handler(void){}
void SysTick_Handler(void){}
void EXTI2_3_IRQHandler(void){}
void EXTI4_15_IRQHandler(void){}
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}


/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles External line 0 to 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
		const uint8_t comp[3] = {0};
		if (!memcmp(zero_cross, comp, sizeof(comp)))
		{
			// Zero Cross just happened
			memset(zero_cross, 1, sizeof(zero_cross));
			
			// Turn all TRIACs off if they shouldn't stay on
			if(dim_trans_buf[0] < AC_DIM_MAX_PERCENT)
			{
				GPIO_ResetBits(GPIOA, GPIO_Pin_1);
			}
			if(dim_trans_buf[1] < AC_DIM_MAX_PERCENT)
			{
				GPIO_ResetBits(GPIOA, GPIO_Pin_2);
			}
			if(dim_trans_buf[2] < AC_DIM_MAX_PERCENT)
			{
				GPIO_ResetBits(GPIOA, GPIO_Pin_3);
			}
			
			// Start the counter from 0 again
			TIM_SetCounter(TIM3, 0);
		}

    // Clear the EXTI line 0 pending bit
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}



// Dim value between 0 (off) and 100 (max)
uint16_t Calc_Dim_CCR(uint32_t dim)
{
	// 10ms is max brightness
	//uint32_t t_cnt_ns = (AC_DIM_PRESCALER + 1) * 1000000000 / SystemCoreClock; // In ns
	//return dim * 100000 / t_cnt_ns;
	return ((100 - dim) * SystemCoreClock) / ((AC_DIM_PRESCALER + 1) * 10000);
}


void TIM3_IRQHandler(void)
{
	// Output compare 1
  if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
		
		if(zero_cross[0])
		{
			zero_cross[0] = 0;
			
			if(dim_trans_buf[0] > AC_DIM_MIN_PERCENT)
			{
				GPIO_SetBits(GPIOA, GPIO_Pin_1);
			}
		}
		if(dim_trans_buf[0] > dim_buf[0])
		{
			TIM_SetCompare1(TIM3, Calc_Dim_CCR(--dim_trans_buf[0]));
		}
		if(dim_trans_buf[0] < dim_buf[0])
		{
			TIM_SetCompare1(TIM3, Calc_Dim_CCR(++dim_trans_buf[0]));
		}
  }
	
	// Output compare 2
  if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

		if(zero_cross[1])
		{
			zero_cross[1] = 0;
			
			if(dim_trans_buf[1] > AC_DIM_MIN_PERCENT)
			{
				GPIO_SetBits(GPIOA, GPIO_Pin_2);
			}
		}
		if(dim_trans_buf[1] > dim_buf[1])
		{
			TIM_SetCompare1(TIM3, Calc_Dim_CCR(--dim_trans_buf[1]));
		}
		if(dim_trans_buf[1] < dim_buf[1])
		{
			TIM_SetCompare1(TIM3, Calc_Dim_CCR(++dim_trans_buf[1]));
		}
  }
	
	// Output compare 3
  if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
		
		if(zero_cross[2])
		{
			zero_cross[2] = 0;
			
			if(dim_trans_buf[0] > AC_DIM_MIN_PERCENT)
			{
				GPIO_SetBits(GPIOA, GPIO_Pin_3);
			}
		}
		if(dim_trans_buf[2] > dim_buf[2])
		{
			TIM_SetCompare1(TIM3, Calc_Dim_CCR(--dim_trans_buf[2]));
		}
		if(dim_trans_buf[2] < dim_buf[2])
		{
			TIM_SetCompare1(TIM3, Calc_Dim_CCR(++dim_trans_buf[2]));
		}
  }
}

void TIM2_IRQHandler(void)
{
	uint16_t ICValue = 0;
	uint16_t DutyCycle = 0;
	uint32_t Frequency = 0;
	
	// Input compare 1
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
	{
		/* Clear TIM2 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

		/* Get the Input Capture value */
		ICValue = TIM_GetCapture1(TIM2);

		if (ICValue != 0)
		{
			/* Duty cycle computation */
			DutyCycle = (TIM_GetCapture1(TIM2) * 100) / ICValue;

			/* Frequency computation */
			Frequency = SystemCoreClock / ICValue;
		}
		else
		{
			DutyCycle = 0;
			Frequency = 0;
		}
	}
	
	// Input compare 2
	if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)
	{
		/* Clear TIM2 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);

		/* Get the Input Capture value */
		ICValue = TIM_GetCapture2(TIM2);

		if (ICValue != 0)
		{
			/* Duty cycle computation */
			DutyCycle = (TIM_GetCapture1(TIM2) * 100) / ICValue;

			/* Frequency computation */
			Frequency = SystemCoreClock / ICValue;
		}
		else
		{
			DutyCycle = 0;
			Frequency = 0;
		}
	}
	
	// Input compare 3
	if (TIM_GetITStatus(TIM2, TIM_IT_CC3) != RESET)
	{
		/* Clear TIM2 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);

		/* Get the Input Capture value */
		ICValue = TIM_GetCapture3(TIM2);

		if (ICValue != 0)
		{
			/* Duty cycle computation */
			DutyCycle = (TIM_GetCapture1(TIM2) * 100) / ICValue;

			/* Frequency computation */
			Frequency = SystemCoreClock / ICValue;
		}
		else
		{
			DutyCycle = 0;
			Frequency = 0;
		}
	}
}

