#include "stm32wb55xx.h"
#include <stdint.h>

#define GPIOBEN (1U << 1)

int main(void)
{
    // 1. Enable clock for GPIO Port B
    RCC->AHB2ENR |= GPIOBEN;

    // 2. Set PB5 as output mode (clear bit 11, set bit 10)
    GPIOB->MODER &= ~(1U << 11);  // Clear bit 11
    GPIOB->MODER |= (1U << 10);   // Set bit 10

    while(1)
    {
        // Set PB5 high
        GPIOB->ODR |= (1U << 5);  // Use bit 5 for PB5

        // Delay
        for(int i = 0; i < 100000; i++){}

        // Set PB5 low
        GPIOB->ODR &= ~(1U << 5);  // Clear bit 5 to turn LED off

        // Delay
        for(int i = 0; i < 100000; i++){}
    }
}
