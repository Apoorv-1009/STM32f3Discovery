//Code uses Timer3 to generate PWM on LED
//Setup:
//LED: PC6

#include "stm32f303xc.h"

void Timer_Initialize()
{
	RCC->APB1ENR |= 1 << 1;   //Enable Timer3
	TIM3->ARR = 65535;   //16 bit number
	TIM3->CCMR1 |= (1 << 5 | 1 << 6);   //PWM Mode 1
	TIM3->CCER |= 1 << 12;   //Enable Output
	TIM3->CR1 |= 1 << 7;   //Enable Auto Re-Load PreLoad (ARPE)
	TIM3->CCR1 = 0;
	TIM3->CCMR1 |= 1 << 3;   //Enable PreLoad
	TIM3->CCER |= 1 << 0;   //Output On PC6
	TIM3->CR1 |= 1 << 0;   //Start Counting
}

void GPIO_Initialize()
{
	RCC->AHBENR |= 1 << 19;   //Enable Clock for Port C
	//Enable AF Mode for PC6:
	GPIOC->MODER |= 1 << 13;
	GPIOC->MODER &= 0 << 12;
	GPIOC->OSPEEDR |= 1 << 12;   //Medium Speed
	//Enable Pull Down:
	GPIOC->PUPDR |= 1 << 13;   
	GPIOC->PUPDR &= 0 << 12;
	GPIOC->AFR[0] = 1 << 25;   
	
}

int main()
{
	Timer_Initialize();
	GPIO_Initialize();
	uint32_t i = 0;
	while(1)
	{
	  i = 0;
		while(i < 65535)
		{
			TIM4->CCR4 = i;
			i++;
		}
	}
	
}

