//Code Generates a delay using Timer3 to Blink LD10(PE13)

#include "stm32f303xc.h"
#include "stm32f3xx.h"
volatile int myTicks = 0;

void Timer_Initialize()
{
	RCC->APB1ENR |= 1 << 3;   //Enable Timer3
	TIM3->PSC = 72;   //Freq = 1Mhz
	TIM3->DIER |= TIM_DIER_UIE;   //UIE: Update Interrupt Enable
	TIM3->EGR |= TIM_EGR_UG;   //UG: Update Generation, Very Important
	 NVIC_EnableIRQ_(TIM3_IRQn);
	
}
void TIM3_IRQ_Handler(void)
{
	myTicks++;
	TIM3->SR &= ~(TIM_SR_UIF);   //UIF: Update Interrupt Flag
}
void delay_ms(int del)
{
	//Enabling Counter to Start counting only when 
	TIM3->CR1 |= TIM_CR1_CEN;   //Enable Timer3 Counting
	myTicks = 0;
	while(myTicks<(del*1000));
	TIM3->CR1 &= ~(TIM_CR1_CEN);   //Disable Timer3 Counting
}


void GPIO_Initialize()
{
	RCC->AHBENR |= 1 << 21;   //Enable Clock for PortE
	GPIOE->MODER |= 1 << 13;   //Enable PE13 as OUTPUT
}


int main()
{
	int delay = 1000;   //Enter Delay here
	while(1)
	{
		GPIOE->BSRR |= 1 << 13;
		delay_ms(delay);
		GPIOE->BSRR |= 1 << (13+16);
		delay_ms(delay);
	}
}
