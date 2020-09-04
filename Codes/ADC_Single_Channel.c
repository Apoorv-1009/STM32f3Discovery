//Code reads ADC value from a joystick on PA2 and displays it using PWM on a LED on PC6
//Setup:
//LED: PC6
//Joystick: PA2

#include "stm32f303xc.h"
volatile int myTicks = 0;

void SysTick_Initialize()
{
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000);
}

void SysTick_Handler()
{
	myTicks++;
}

void delay_ms(int del)
{
	myTicks = 0;
	while(myTicks < 0);
}

void Timer_Initialize()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;   //Enable Timer3
	TIM3->CCER |= TIM_CCER_CC1E;   //Enable Channel 1
	TIM3->CR1 |= TIM_CR1_ARPE;   //Enable Auto Re-Load PreLoad (ARPE)
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE;  //Enable PreLoad
	TIM3->CCER |= TIM_CCER_CC1E;   //Enable Timer3 as OUTPUT
	TIM3->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);   //PWM Mode 1
	
	TIM3->ARR = 4095;   //12 bit number
	TIM3->CCR1 = 0;
	TIM3->PSC |= 1;   //Prescaler
	
	TIM3->EGR |= TIM_EGR_UG;   //Update Registers
	TIM3->CR1 |= TIM_CR1_CEN;   //Start Counting
}


void ADC_Initialize()
{
	RCC->CFGR2 |= RCC_CFGR2_ADCPRE12_DIV1;   //f/1 = 72Mhz
	RCC->AHBENR |= RCC_AHBENR_ADC12EN;   //Enable ADC Clock
	
	ADC1->CR &= ~(ADC_CR_ADVREGEN_1);   //Enable Voltage Regulator 
	ADC1->CR |= (ADC_CR_ADVREGEN_0);
	delay_ms(10);
	
	ADC1_2_COMMON->CCR |= 1 << 16;   //Set ADC Clock to Synchronous Mode
	ADC1->CR |= ~(ADC_CR_ADCALDIF);   //Launch Calibration in Single-Ended Input Mode
	ADC1->CR |= ADC_CR_ADCAL;   //Start Calibration
	while( (ADC1->CR) & ADC_CR_ADCAL);   //Wait till Calibration is Complete
	delay_ms(10);
	
	ADC1->CR |= ADC_CR_ADEN;   //Enable ADC
	while(!( (ADC1->ISR) & ADC_ISR_ADRDY) );   //Wait till ADC is Ready
	
	//Set channel you want to convert in the sequence registers:
	ADC1->SQR1 |= (ADC_SQR1_SQ1_1 | ADC_SQR1_SQ1_0);   //Channel 3, Sequence 1
	
	//Set Sampling Rate:
	ADC1->SMPR1 |= ( ADC_SMPR1_SMP3_0 | ADC_SMPR1_SMP3_1 | ADC_SMPR1_SMP3_2 );   //601.5 ADC Clock Cycles
	
	ADC1->CFGR |= ADC_CFGR_CONT;   //Enable Continuous Conversion Mode
	ADC1->CR |= ADC_CR_ADSTART;   //ADC  Start of Regular Conversion
}


void GPIO_Initialize()
{
	//Setup PC6:
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;   //Enable Clock for Port C
	//Enable AF Mode for PC6:
	GPIOC->MODER &= ~(GPIO_MODER_MODER6_0);
	GPIOC->MODER |= GPIO_MODER_MODER6_1;
	GPIOC->AFR[0] |= 1 << 25;   //AF2
	
	//Setup PA2:
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;   //Enable Clock for Port A
	GPIOA->MODER |= GPIO_MODER_MODER2;   //Enable Analog Mode
}


int main()
{
	SysTick_Initialize();
	Timer_Initialize();
	ADC_Initialize();
	while(1)
	{
		TIM3->CCR1 = ADC1->DR;
	}
}
