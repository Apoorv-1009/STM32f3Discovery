/*
Code implements Motor Code and Hill Assist on LEDs thru PWM with Timer1, by reading Joystick values
Onboard Button has been used to implement HillAssist
Setup:
All LEDs are Onboard
Button: PA0(Onboard)
PWM LEDs: LD3(PE9->Ch1), LD7(PE11->Ch2)
Direction LEDs: LD6(PE15), LD10(PE13)
Joystick: PA1(Ch 2), PA2(Ch 3)
x->PA1   y->PA2

      Left     	     Right
LED:  LD10(PE13)     LD6(PE15)
PWM:  LD7(PE11)      LD3(PE9)
*/


#include "stm32f303xc.h"
#include "stdlib.h"

volatile int myTicks = 0;
volatile uint16_t samples[2] = {0,0};
//x -> samlpes[0]
//y -> samples[1]

	
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
	//Enable Clocks:
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;   //Enable Clock for Port A
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN;   //Enable Clock for Port E
	
	//Setup PA0:
	GPIOA->MODER &= ~(GPIO_MODER_MODER0);   //Enable as INPUT
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1;   //Enable Pull-Down
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR0_0);
	
	//Setup PA1:
	GPIOA->MODER |= GPIO_MODER_MODER1_1 | GPIO_MODER_MODER1_0;   //Enable Analog Mode
	
	//Setup PA2:
	GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER2_0;   //Enable Analog Mode

	//Setup PE15:
	GPIOE->MODER &= ~(GPIO_MODER_MODER15_1);   //Enable as OUTPUT
	GPIOE->MODER |= GPIO_MODER_MODER15_0;
	GPIOE->OTYPER &= ~(GPIO_OTYPER_OT_15);   //Output Push-Pull
	GPIOE->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR15);   //High Speed
	
	//Setup PE13:
	GPIOE->MODER &= ~(GPIO_MODER_MODER13_1);   //Enable as OUTPUT
	GPIOE->MODER |= GPIO_MODER_MODER13_0;
	GPIOE->OTYPER &= ~(GPIO_OTYPER_OT_13);   //Output Push-Pull
	GPIOE->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR13);   //High Speed
	
	//Setup PE9:
	//Enable AF Mode:
	GPIOE->MODER |= GPIO_MODER_MODER9_1;   
	GPIOE->MODER &= ~(GPIO_MODER_MODER9_0);
	GPIOE->AFR[1] |= 1 << 5;
	
	//Setup PE11:
	//Enable AF Mode:
	GPIOE->MODER |= GPIO_MODER_MODER11_1;   
	GPIOE->MODER &= ~(GPIO_MODER_MODER11_0);
	GPIOE->AFR[1] |= 1 << 13;
}


void Timer_Initialize()
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;   //Enable Timer1
	TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;   //Enable Channel 1 & 2 as OUTPUT
	TIM1->CR1 |= TIM_CR1_ARPE;   //Enable Auto Re-Load Preload (ARPE)
	
	TIM1->CCMR1 |= TIM_CCMR1_OC1PE;   //Enable PreLoad for Channel 1
	TIM1->CCMR1 |= TIM_CCMR1_OC2PE;   //Enable PreLoad for Channel 2 
	
	TIM1->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);   //PWM Mode 1 for Channel 1
	TIM1->CCMR1 |= (TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2);   //PWM Mode 1 for Channel 2  
	
	TIM1->PSC = 1;   //freq/1 = 72 Mhz
	TIM1->ARR = 4095;   //16 Bit Value
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	
	TIM1->BDTR |= TIM_BDTR_MOE;   //Main Output Enable
	
	TIM1->EGR |= TIM_EGR_UG;   //Update Registers
	TIM1->CR1 |= TIM_CR1_CEN;   //Start Counting
	 
}


void ADC_Initialize()
{
	RCC->CFGR |= RCC_CFGR2_ADCPRE12_DIV1;   //f/1 = 72Mhz
	RCC->AHBENR |= RCC_AHBENR_ADC12EN;   //Enable ADC Clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;   //Enable DMA1  
	
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
	
	ADC1->SQR1 |= 1 << 0;   //Set Length of 2 ADC Sequences  
	//Set channel you want to convert in the sequence registers:
	ADC1->SQR1 |= ADC_SQR1_SQ1_1;   //Channel 2, Sequence 1  
	ADC1->SQR1 |= ADC_SQR1_SQ2_1 | ADC_SQR1_SQ2_0;	  //Channel 3, Sequence 2  
	
	ADC1->CFGR |= ADC_CFGR_DMAEN;   //DMA Enable
	ADC1->CFGR |= ADC_CFGR_DMACFG;   //Circular Mode seleced
		 
	//DMA Settings:  
	DMA1_Channel1->CPAR = (uint32_t)(&(ADC1->DR));   //Give location to read from  
	DMA1_Channel1->CMAR = (uint32_t)samples;   //Give location to store  
	DMA1_Channel1->CNDTR |= 2;   //Define Number of times to transfer data 
	
	DMA1_Channel1->CCR |= DMA_CCR_CIRC;   //Enable Circular Mode
	DMA1_Channel1->CCR |= DMA_CCR_MINC;   //Memory Increment Mode
	DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;   //Define Peripheral Data size as 16 bits
	DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;   //Define Memory size as 16 bits
	
	DMA1_Channel1->CCR |= DMA_CCR_EN;   //Enable DMA1
	//!NOTE! Do NOT Enable DMA before you define all of its settings
	
	//Set Sampling Rate:
	ADC1->SMPR1 |= ( ADC_SMPR1_SMP2);   //601.5 Sampling Rate for Channel 2
	ADC1->SMPR1 |= ( ADC_SMPR1_SMP3);   //601.5 Sampling Rate for Channel 3   
																																							 
	ADC1->CFGR |= ADC_CFGR_CONT;   //Enable Continuous Conversion Mode
	ADC1->CR |= ADC_CR_ADSTART;   //ADC  Start of Regular Conversion
	delay_ms(10);
}


//        |Left    Right| |      Left PWM       |  |      Right PWM     | |  Values  |   
void Drive(int DL, int DR, int oct0, int a, int b, int oct1, int p, int q, int X, int Y)
{
	if(DL == 1)
		GPIOE->BSRR |= 1 << 13;   //Turn on LEFT LED
	else
		GPIOE->BRR |= 1 << 13;   //Turn off LEFT LED
	
  if(DR == 1)
		GPIOE->BSRR |= 1 << 15;   //Turn on RIGHT LED
	else
		GPIOE->BRR |= 1 << 15;   //Turn off RIGHT LED
	
  TIM1->CCR2 = (uint32_t) abs(4095*oct0 - abs(X*a) - abs(Y*b));   //Left PWM
  TIM1->CCR1 = (uint32_t) abs(4095*oct1 - abs(X*p) - abs(Y*q));   //Right PWM
	
	//delay_ms(5);
}


int mapp(float k, float l, float h, float L, float H)
{
	return ((k - l)/(h - l))*(H - L) + L;
}

int read(int k)
{
	int val = 0;
	val = samples[k];
	val = mapp(val, 0, 4095, -4095, 4095);
	
	if(abs(val) < 400)
		val = 0;
	
	if(val < -3900)
		val = -4095;
	
	if(val > 3900)
		val = 4095;
	
	return val;
}

int Toggle(int a)
{
	if(a == 0)
		a = 1;
	
	else
		a = 0;
		
	return a;
}

void MotorCode(int x, int y)
{
		if(abs(x) < 20 && abs(y) < 20)   //No Motion
			Drive(0,0,0,0,0,0,0,0,0,0);
		
		else if(abs(x) < 10 && y < 0)   //Full Forward
			Drive(1,1,0,0,1,0,0,1,x,y);
		
		else if(abs(x) < 10 && y > 0)   //Full Backward
			Drive(0,0,0,0,1,0,0,1,x,y);
		
		else if(x < 0 && abs(y) <= 20)   //Spot Turn Left
			Drive(0,1,0,1,0,0,1,0,x,y);
		
		else if(x > 0 && abs(y) <= 20)   //Spot Turn Right
			Drive(1,0,0,1,0,0,1,0,x,y);
		
		else if(x > 0 && y < 0 && x >= abs(y))   //Octet 1
		{
			if(abs(x) > 4095 - abs(y))   
				Drive(1,0,0,1,0,1,0,1,x,y);
			else   
				Drive(1,0,1,0,1,0,1,0,x,y);
		}
				
		else if(x > 0 && y < 0 && x < abs(y))   //Octet 2
		{
			if(abs(y) > 4095 - abs(x))
				Drive(1,1,0,0,1,1,1,0,x,y);
			else
				Drive(1,1,1,1,0,0,0,1,x,y);
		}
		
		
		else if(x < 0 && y < 0 && abs(y) > abs(x))   //Octet 3
		{
			if(abs(y) > 4095 - abs(x))
				Drive(1,1,1,1,0,0,0,1,x,y);
			else
				Drive(1,1,0,0,1,1,1,0,x,y);
		}
		
		else if(x < 0 && y < 0 && abs(x) >= abs(y))   //Octet 4
		{
			if(abs(x) > 4095 - abs(y))
				Drive(0,1,1,0,1,0,1,0,x,y);
			else
				Drive(0,1,0,1,0,1,0,1,x,y);
		}
		
		
		else if(x < 0 && y > 0 && abs(x) > abs(y))   //Octet 5
		{
			if(abs(x) > 4095 - abs(y))
				Drive(0,1,0,1,0,1,0,1,x,y);
			else
				Drive(0,1,1,0,1,0,1,0,x,y);
		}
		
		else if(x < 0 && y > 0 && abs(y) >= abs(x))   //Octet 6
		{
			if(abs(y) > 4095 - abs(x)) 
				Drive(0,0,0,0,1,1,1,0,x,y);
			else
				Drive(0,0,1,1,0,0,0,1,x,y);
		}
		
		else if(x > 0 && y > 0 && abs(y) >= abs(x))   //Octet 7
		{
			if(abs(y) > 4095 - abs(x))
				Drive(0,0,1,1,0,0,0,1,x,y);
			else
				Drive(0,0,0,0,1,1,1,0,x,y);
		}
		
		else if(x > 0 && y > 0 && abs(x) > abs(y))   //Octet 8
		{
			if(abs(x) > 4095 - abs(y))
				Drive(1,0,1,0,1,0,1,0,x,y);
			else
				Drive(1,0,0,1,0,1,0,1,x,y);
		}
		
		//Test Drive:
		//Drive(1,1,0,1,0,0,0,1,x,y);
}


int main()
{
	SysTick_Initialize();
	GPIO_Initialize();
	Timer_Initialize();
	ADC_Initialize();
	
	int x_read,y_read;
	int button = 0;
	int prev_button = 0;
	int k = 0;
	while(1)
	{
		x_read = read(0);   //Read mapped x co-ordinate of Joystick
		y_read = read(1);   //Read mapped y co-ordinate of Joystick
		button = (GPIOA->IDR & GPIO_IDR_0);
		if(button && !(prev_button))   //Button Press
		{
			k = Toggle(k);
			prev_button = 1;
		}
		else
		{
			prev_button = (GPIOA->IDR & GPIO_IDR_0);
		}
		
		switch(k)
		{
			case 1: break;
			
			case 0: MotorCode(x_read,y_read);
				break;
			
			default: delay_ms(1);
		}
		
	}
}


