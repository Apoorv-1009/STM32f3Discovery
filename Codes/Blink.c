//Code Blinks an On-Board LED LD10(PE13)


#include "stm32f303xc.h"

void GPIO_Initialize()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;   //Enable Clock for PortE
	
	GPIOE->MODER |= GPIO_MODER_MODER13_0;   //Enable PE13 as OUTPUT
	GPIOE->MODER &= ~(GPIO_MODER_MODER13_0);
	
	GPIOE->OTYPER &= ~(GPIO_OTYPER_OT_13);   //Output Push-Pull
	GPIOE->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR13);   //High Speed
}

void delay(int k)
{
	int i = 0;
	for(i = 0; i < k; i++);
}

int main()
{
	GPIO_Initialize();
	
	
	while(1)
	{
		GPIOE->BSRR |= 1<<13;   //Turn on PE13
		delay(1000000);
		GPIOE->BSRR |= 1<<(13+16);   //Turn off PE13
		delay(1000000);
	}
}

