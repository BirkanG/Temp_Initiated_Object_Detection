#include <stdint.h>
#include <stdio.h>
#include "TM4C123GH6PM.h"

void ADC1_Init(void) {
    SYSCTL->RCGCADC |= 0x02;    // Enable ADC1 clock
    SYSCTL->RCGCGPIO |= 0x10;   // Enable Port E clock
    while ((SYSCTL->PRGPIO & 0x10) == 0); // Wait for GPIOE clock to stabilize
		while ((SYSCTL->PRADC & 0x02) == 0);
    GPIOE->AFSEL |= 0x02;       // Enable alternate function on PE1
    GPIOE->PCTL &= ~0x000000F0;
    GPIOE->AMSEL |= 0x02;       // Enable analog functionality on PE1
    GPIOE->DEN &= ~0x02;        // Disable digital on PE1

    ADC1->ACTSS &= ~0x08;       // Disable sequencer 3
    ADC1->EMUX &= ~0xF000;      // Software trigger
    ADC1->SSMUX3 = 0x0000;        // AIN0 (PE3)
    ADC1->SSCTL3 = 0x0006;        // IE0 and END0
    ADC1->PC = 0x01;            // 125ksps
    ADC1->ACTSS |= 0x08;        // Enable sequencer 3
}

int ADC1_Read(void) {
    ADC1->PSSI = 0x08;          // Start sampling
    while ((ADC1->RIS & 0x08) == 0); // Wait for completion
    int result = ADC1->SSFIFO3; // Read result
    ADC1->ISC = 0x08;           // Clear interrupt flag
    return result;
}

float ConvertToVoltage(int adcValue) {
    // Convert ADC value to voltage value
    return (adcValue * 3.3 / 4096.0);
}
