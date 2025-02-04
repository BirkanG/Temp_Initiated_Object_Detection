#include "TM4C123GH6PM.h"

void LED_init(void) {
//    SYSCTL->RCGCGPIO |= 0x01;        // Enable clock for Port C
//    while ((SYSCTL->PRGPIO & 0x01) == 0);  // Wait until Port C is ready
    
    GPIOC->DIR |= 0x20;             // Set PC5 as output
    GPIOC->AFSEL &= ~0x20;          // Disable alternate functions on PC5
    GPIOC->PCTL &= ~0x00500000;     // Clear PCTL for PC5
    GPIOC->AMSEL &= ~0x20;          // Disable analog on PC5
    GPIOC->DEN |= 0x20;             // Enable digital on PC5
}

void turn_on_LED(void) {
    GPIOC->DATA |= 0x20;     // Set PC5 high (turn on LED)
}

void turn_off_LED(void) {
    GPIOC->DATA &= ~0x20;     // Set PC5 low (turn off LED)
}