//Code Blinks an On-Board LED LD10(PE13)


#include "stm32f303xc.h"

void GPIO_Initialize()
{
	RCC->AHBENR |= 1 << 21;   //Enable Clock for PortE
	GPIOE->MODER |= 1 << 13;   //Enable PE13 as OUTPUT
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

