//Code Turns on LED LD10(PE13) when a Button is pushed
//Setup:
//Button: A0 (onboard)
//LED: LD10(PE13) (onboard)

#include "stm32f303xc.h"

void GPIO_Initialize()
{
	RCC->AHBENR |= 1 << 21;   //Enable Clock for PortE
	GPIOE->MODER |= 1 << 13;   //Enable PE13 as OUTPUT
	
	RCC->AHBENR |= 1 << 17;   //Enable Clock for PortA
	GPIOE->MODER &= (0 << 0 | 0 << 1);   //Enable PA0 as INPUT
	//Enable Pull-Down
	GPIOE->PUPDR |= 1 << 1;
	GPIOE->PUPDR &= 0 << 0;
	
}

int main()
{
	GPIO_Initialize();
	int pressed = 0;
	while(1)
	{
		pressed = GPIOA->IDR & GPIO_IDR_0;
		
		if(pressed)
			GPIOE->BSRR |= 1<<13;
		
		else
			GPIOE->BSRR |= 1<<(13+16);
		
	}
	
}

