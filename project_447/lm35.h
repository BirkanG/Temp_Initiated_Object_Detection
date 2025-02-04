#include <stdio.h>
#include <stdint.h>
#include "TM4C123GH6PM.h"
#include "deep_sleep.h"

//uint8_t tempExceeded = 0; // Flag for threshold exceeded

void init_Comparator0(void) {
    // Enable clock for GPIOC and Comparator module
    SYSCTL->RCGCGPIO |= (1 << 2);  // Enable clock for Port C&F
    SYSCTL->RCGCACMP |= (1 << 0);   // Enable clock for Comparator module
	  SYSCTL->SCGCACMP |= (1 << 0);        // Keep comparator clock active in deep sleep
		SYSCTL->SCGCGPIO |= (1 << 2); // Keep GPIOC clock active in deep sleep
		
    while ((SYSCTL->PRGPIO & (1 << 2)) == 0); // Wait for GPIOC clock to stabilize

    // Configure PC6 and PC7 as analog inputs
    GPIOC->DIR &= ~((1 << 6) | (1 << 7)); // Set PC6 and PC7 as inputs
    GPIOC->DEN &= ~((1 << 6) | (1 << 7)); // Disable digital functionality
    GPIOC->AFSEL |= (1 << 6) | (1 << 7);  // Enable alternate functions
    GPIOC->AMSEL |= (1 << 6) | (1 << 7);  // Enable analog functionality
		
	  // Configure PF1
//		GPIOF->LOCK = 0x4C4F434B; // Unlock GPIOF
//    GPIOF->CR |= (1 << 1);
//    GPIOF->LOCK = 0;
//    GPIOF->DIR |= (1 << 1);
//    GPIOF->DEN |= (1 << 1);
//    GPIOF->DATA &= ~(1 << 1); // Turn off red LED

    // Configure Comparator 0
    COMP->ACREFCTL &= ~0x01;            // Disable internal voltage reference
	  COMP->ACCTL0 = 0x00000010;
    // Enable Comparator Interrupt
    COMP->ACINTEN |= (1 << 0);          // Enable interrupts for Comparator 0
		NVIC_EnableIRQ(COMP0_IRQn); // Enable in NVIC
}

void COMP0_Handler(void) {
    COMP->ACMIS = 0x01;  // Clear interrupt flag
    for (int i = 0; i < 100000; i++);  // Delay loop
    if ((COMP->ACSTAT0 & 0x02) == 0x02) {  // Check comparator output
        COMP->ACINTEN &= ~0x1;  // Disable comparator interrupt
        in_deep_sleep = 0;
    }
}

//int main(void) {
//		init_Comparator0();
//    while (1) {
//			  for (volatile uint32_t i = 0; i < 1000000; i++){};
//        if (tempExceeded) {
//            tempExceeded = 0;
//					
//            char message[100];
//            sprintf(message, "Threshold exceeded!\n\r\4"); // Debug message
//            OutStr(message);

//            GPIOF->DATA ^= (1 << 1);  // Toggle red LED
//        }
//        EnterDeepSleep();
//    }

//    return 0;
//}