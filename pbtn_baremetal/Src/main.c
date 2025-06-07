#include "stm32wb55xx.h"
#include <stdint.h>

#define GPIOBEN (1U << 1)
#define GPIODEN (1U << 3)

int main(void)
{
    // 1. Enable clock for GPIO Port B and GPIO Port D
	RCC->AHB2ENR |= GPIOBEN;
	RCC->AHB2ENR |= GPIODEN;

	// 2. Set PB5 as output mode (clear bit 11, set bit 10)
	GPIOB->MODER &= ~(1U << 11);  // Clear bit 11
	GPIOB->MODER |= (1U << 10);   // Set bit 10

	//3.set PDO as input
	GPIOD->MODER &= ~(1U << 0);
	GPIOD->MODER &= ~(1U << 1);  /* or we can do in single line as 01 is binary of 3 GPIOD->MODER &= ~(3U << 0);  // Clear bits 0 and 1*/

	//4. Enable pull-up resistor for PD0
	GPIOD->PUPDR &= ~(3U << 0);  // Clear bits 0 and 1 first
	GPIOD->PUPDR |= (1U << 0);   // Set bit 0 for pull-up


    while(1)
    {
        // Set PB5 high

    	 if ((GPIOD->IDR & (1U << 0)) == 0)
    	 {
    	        // Button is pressed, turn LED on (PB5 high)
    	        GPIOB->ODR |= (1U << 5);
    	 }
        //GPIOB->ODR |= (1U << 5);  // Use bit 5 for PB5

        // Delay
        for(int i = 0; i < 100000; i++){}

        // Set PB5 low
        //GPIOB->ODR &= ~(1U << 5);  // Clear bit 5 to turn LED off

        // Delay
        //for(int i = 0; i < 100000; i++){}
    }
}
