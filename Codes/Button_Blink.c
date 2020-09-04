//Code Turns on LED LD10(PE13) when a Button is pushed
//Setup:
//Button: A0 (onboard)
//LED: LD10(PE13) (onboard)

#include "stm32f303xc.h"

void GPIO_Initialize()
{
	//PE13 Setup:
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN;   //Enable Clock for PortE
	
	GPIOE->MODER &= ~(GPIO_MODER_MODER13_1);   //Enable PE13 as OUTPUT
	GPIOE->MODER |= GPIO_MODER_MODER13_0;   
	
	GPIOE->OTYPER |= ~(GPIO_OTYPER_OT_13);   //Output Push-Pull
	GPIOE->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR13;   //High Speed 
	
	//PA0 Setup:
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;   //Enable Clock for PortA
	GPIOE->MODER &= ~(GPIO_MODER_MODER0);   //Enable PA0 as INPUT
	
	GPIOE->PUPDR |= GPIO_PUPDR_PUPDR0_1;   //Enable Pull-Down
	GPIOE->PUPDR &= ~(GPIO_PUPDR_PUPDR0_0);
	
}

int main()
{
	GPIO_Initialize();
	int pressed = 0;
	while(1)
	{
		pressed = GPIOA->IDR & GPIO_IDR_0;
		
		if(pressed)
			GPIOE->BSRR |= 1 << 13;
		
		else
			GPIOE->BSRR |= 1 << (13+16);
		
	}
	
}


