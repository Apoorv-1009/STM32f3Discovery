//Code uses Timer3 to generate PWM on LED
//Setup:
//LED: PC6

#include "stm32f303xc.h"

void Timer_Initialize()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;   //Enable Timer3
	TIM3->CCER |= TIM_CCER_CC1E;   //Enable Channel 1
	TIM3->CR1 |= TIM_CR1_ARPE;   //Enable Auto Re-Load PreLoad (ARPE)
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE;  //Enable PreLoad
	TIM3->CCER |= TIM_CCER_CC1E;   //Enable Timer3 as OUTPUT
	TIM3->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);   //PWM Mode 1
	
	TIM3->ARR = 65535;   //16 bit number
	TIM3->CCR1 = 0;
	TIM3->PSC |= 1;   //Prescaler
	
	TIM3->EGR |= TIM_EGR_UG;   //Update Registers
	TIM3->CR1 |= TIM_CR1_CEN;   //Start Counting
}


void GPIO_Initialize()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;   //Enable Clock for Port C
	//Enable AF Mode for PC6:
	GPIOC->MODER &= ~(GPIO_MODER_MODER6_0);
	GPIOC->MODER |= GPIO_MODER_MODER6_1;
	GPIOC->AFR[0] |= GPIO_AFRL_AFRL2;   //AF2
	
}

int main()
{
	Timer_Initialize();
	GPIO_Initialize();
	uint32_t i = 0;
	while(1)
	{
	  i = 0;
		while(i < TIM3->ARR)
		{
			TIM3->CCR1 = i;
			i++;
		}
	}
	
}
