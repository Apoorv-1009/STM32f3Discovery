//Code Generates a delay in ms using Timer3 to Blink LD10(PE13)

#include "stm32f303xc.h"
volatile int myTicks = 0;

void Timer_Initialize()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;   //Enable Timer3
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
	TIM3->CR1 |= TIM_CR1_CEN;   //Start Timer3 Counting
	myTicks = 0;
	while(myTicks<(del*1000));
	TIM3->CR1 &= ~(TIM_CR1_CEN);   //Stop Timer3 Counting
}


void GPIO_Initialize()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN;   //Enable Clock for PortE
	
	GPIOE->MODER |= GPIO_MODER_MODER13_0;   //Enable PE13 as OUTPUT
	GPIOE->MODER |= ~(GPIO_MODER_MODER13_1);
	
	GPIOE->OTYPER &= ~(GPIO_OTYPER_OT_13);   //Output Push-Pull
	GPIOE->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR13);   //High Speed
}


int main()
{
	GPIO_Initialize();
	Timer_Initialize();
	int delay = 1000;   //Enter Delay in ms here
	while(1)
	{
		GPIOE->BSRR |= 1 << 13;
		delay_ms(delay);
		GPIOE->BSRR |= 1 << (13+16);
		delay_ms(delay);
	}
}
