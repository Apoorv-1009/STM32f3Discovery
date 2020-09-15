//Code Reads x and y ADC Values from a Joystick and Displays them thru PWM on LEDs using Timer3
//Setup:
//LED: PC6(Ch 1), PA7(Ch 2)
//Joystick: PA1(Ch 1), PA0(Ch 2)

#include "stm32f303xc.h"

volatile int myTicks = 0;
uint16_t samples[2] = {0,0};

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
	while(myTicks < del);
}

void GPIO_Initialize()
{
	//PC6 Setup:
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;   //Enable Clock for Port C
	//Enable AF Mode for PC6:
	GPIOC->MODER &= ~(GPIO_MODER_MODER6_0);
	GPIOC->MODER |= (GPIO_MODER_MODER6_1);
	GPIOC->AFR[0] |= 1<<25;   //AF2
	
	//PA7 Setup:
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;   //Enable Clock for Port A
	//Enable AF Mode for PA7:
	GPIOA->MODER &= ~(GPIO_MODER_MODER7_0);
	GPIOA->MODER |= (GPIO_MODER_MODER7_1);
	GPIOA->AFR[0] |= 1<<29;   //AF2
	
	//Setup PA1:
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;   //Enable Clock for Port A
	GPIOA->MODER |= GPIO_MODER_MODER1;   //Enable Analog Mode
	
	//Setup PA0:
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;   //Enable Clock for Port A
	GPIOA->MODER |= GPIO_MODER_MODER0;   //Enable Analog Mode
}

void Timer_Initialize()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;   //Enable Timer3
	TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;   //Enable Channel 1 & 2 as OUTPUT
	TIM3->CR1 |= TIM_CR1_ARPE;   //Enable Auto Re-Load Preload (ARPE)
	
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE;   //Enable PreLoad for Channel 1
	TIM3->CCMR1 |= TIM_CCMR1_OC2PE;   //Enable PreLoad for Channel 2  ~~~~~
	
	TIM3->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);   //PWM Mode 1 for Channel 1
	TIM3->CCMR1 |= (TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2);   //PWM Mode 1 for Channel 2  
	
	TIM3->PSC = 1;   //freq/1 = 72 Mhz
	TIM3->ARR = 65535;   //16 Bit Value
	TIM3->CCR1 = 0;
	TIM3->CCR2 = 0;
	
	TIM3->EGR |= TIM_EGR_UG;   //Update Registers
	TIM3->CR1 |= TIM_CR1_CEN;   //Start Counting
}


void ADC_Initialize()
{
	RCC->CFGR |= RCC_CFGR2_ADCPRE12_DIV1;   //f/1 = 72Mhz
	RCC->AHBENR |= RCC_AHBENR_ADC12EN;   //Enable ADC Clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;   //Enable DMA1  !!!!!
	
	//Enable Voltage Regulator
	ADC1->CR &= ~(ADC_CR_ADVREGEN_1);
	ADC1->CR |= ADC_CR_ADVREGEN_0;
	delay_ms(10);
	
	ADC1_2_COMMON->CCR |= 1 << 16;   //Set ADC Clock to Synchronous Mode
	ADC1->CR &= ~(ADC_CR_ADCALDIF);   //Launch Calibration in Single-Ended Input Mode
	ADC1->CR |= ADC_CR_ADCAL;   //Start Calibration
	while( (ADC1->CR) & ADC_CR_ADCAL);   //Wait till Calibration is Complete
	delay_ms(10);
	
	ADC1->CR |= ADC_CR_ADEN;   //Enable ADC
	//while(!( (ADC1->ISR) & ADC_ISR_ADRDY) );   //Wait till ADC is Ready
	
	ADC1->SQR1 |= 1 << 0;   //Set Length of 2 ADC Sequences  !!!!!
	//Set channel you want to convert in the sequence registers:
	ADC1->SQR1 |= ADC_SQR1_SQ1_0;   //Channel 1, Sequence 1  !!!!!
	ADC1->SQR1 |= ADC_SQR1_SQ2_1;	  //Channel 2, Sequence 2  !!!!!
	
	ADC1->CFGR |= ADC_CFGR_DMAEN;   //DMA Enable
	ADC1->CFGR |= ADC_CFGR_DMACFG;   //Circular Mode seleced
		 
	//DMA Settings:  !!!!!
	DMA1_Channel1->CPAR = (uint32_t)(&(ADC1->DR));   //Give location to read from  !!!!!
	DMA1_Channel1->CMAR = (uint32_t)samples;   //Give location to store  !!!!!
	DMA1_Channel1->CNDTR |= 2;   //Define Number of times to transfer data !!!!!
	
	DMA1_Channel1->CCR |= DMA_CCR_CIRC;   //Enable Circular Mode
	DMA1_Channel1->CCR |= DMA_CCR_MINC;   //Memory Increment Mode
	DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;   //Define Peripheral Data size as 16 bits
	DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;   //Define Memory size as 16 bits
	
	DMA1_Channel1->CCR |= DMA_CCR_EN;   //Enable DMA1
	//!NOTE! Do NOT Enable DMA before you define all of its settings
	
	//Set Sampling Rate:
	ADC1->SMPR1 |= ( ADC_SMPR1_SMP1);   //601.5 Sampling Rate for Channel 1
	ADC1->SMPR1 |= ( ADC_SMPR1_SMP2);   //601.5 Sampling Rate for Channel 2   !!!!!
																																							 
	ADC1->CFGR |= ADC_CFGR_CONT;   //Enable Continuous Conversion Mode
	ADC1->CR |= ADC_CR_ADSTART;   //ADC  Start of Regular Conversion
	delay_ms(10);
}

int main()
{
	SysTick_Initialize();
	GPIO_Initialize();
	Timer_Initialize();
	ADC_Initialize();
	
	while(1)
	{
		TIM3->CCR1 = samples[0];
		TIM3->CCR2 = samples[1];
	}
	
}
