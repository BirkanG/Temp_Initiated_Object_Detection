#include "TM4C123GH6PM.h"
volatile int interrupt_count = 0; // Counter for interrupt occurrences
volatile int high_time = 120;              // High time for the pulse (1us units)
volatile int low_time = 80;                // Low time for the pulse (1us units)
volatile int duration;
volatile int min_digital_temp_thr = 25;				
volatile int digital_temp_thr = 30;   // digital temp threshold value start with 30 
void init_speaker(void) {
    volatile int *NVIC_EN0 = (volatile int *)0xE000E100;
    volatile int *NVIC_PRI4 = (volatile int *)0xE000E410;

    GPIOC->DIR |= 0x10;        // Set PC4 as output
    GPIOC->AFSEL &= ~0x10;     // Regular port function
    GPIOC->PCTL &= ~0x000F0000; // No alternate function
    GPIOC->AMSEL &= ~0x10;     // Disable analog
    GPIOC->DEN |= 0x10;        // Enable port digital

    SYSCTL->RCGCTIMER |= 0x01; // Start Timer0
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");

    TIMER0->CTL &= ~0x01;      // Disable timer during setup
    TIMER0->CFG = 0x04;        // Set 16-bit timer mode
    TIMER0->TAMR = 0x02;       // Set to periodic, count down
    TIMER0->TAILR = low_time;  // Set interval load as LOW
    TIMER0->TAPR = 15;         // Divide clock by 16 to get 1us
    TIMER0->IMR = 0x01;        // Enable timeout interrupt

    *NVIC_PRI4 &= 0x00FFFFFF;  // Clear interrupt 19 priority
    *NVIC_PRI4 |= 0x40000000;  // Set interrupt 19 priority to 2

    *NVIC_EN0 |= 0x00080000;   // Enable interrupt 19 (Timer0A)
		TIMER0->IMR &= ~0x01;
    //TIMER0->CTL |= 0x01;       // Enable timer
}

void TIMER0A_Handler(void) {
    TIMER0->ICR = 0x01;         // Clear the interrupt flag
	
		duration = 10000/((digital_temp_thr - min_digital_temp_thr) +1);
		high_time = duration*0.6;
		low_time =  duration*0.4;
	
    GPIOC->DATA ^= 0x10;        // Toggle PC4

    // Increment the interrupt counter
//    interrupt_count++;

//    // Disable the interrupt after 2 seconds
//    if (interrupt_count >= 15000) { // 2 seconds = 2000 interrupts at 1ms intervals
//        TIMER0->IMR &= ~0x01;   // Disable Timer0A interrupt
//				TIMER0->CTL &= ~0x01;      // Disable timer during setup
//    }
		
    // Adjust Timer Interval Load (TAILR) for the next cycle
    if (GPIOC->DATA & 0x10) {   // If PA2 is HIGH
        TIMER0->TAILR = high_time; // Set HIGH duration
				
    } else {
        TIMER0->TAILR = low_time;  // Set LOW duration
    }
}

void speaker_activate(void) {
		TIMER0->IMR |= 0x01;
    TIMER0->CTL |= 0x01;       // Enable timer
}