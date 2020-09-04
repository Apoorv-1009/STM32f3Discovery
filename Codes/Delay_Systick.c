//Code generates a delay in ms using Systick Timer to blink LD10(PE13)

#include "stm32f303xc.h"
#include "stm32f3xx.h"
volatile int myTicks = 0;

void SysTick_Initialize()
{
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000);
}

void SysTick_Handler(void)
{
	myTicks++;
}
void delay_ms(int del)
{
	myTicks = 0;
	while(myTicks<del);
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
	SysTick_Initialize();
	GPIO_Initialize();
	
	int delay = 1000;   //Enter Delay in ms here
	while(1)
	{
		GPIOE->BSRR |= 1 << 13;
		delay_ms(delay);
		GPIOE->BSRR |= 1 << (13+16);
		delay_ms(delay);
	}
}
