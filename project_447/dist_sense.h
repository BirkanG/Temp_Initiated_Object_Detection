#include "TM4C123GH6PM.h"

#define TRIG_PIN 5  // PB5 for TRIG signal
#define ECHO_PIN 4  // PB4 for ECHO signal

//void setup_gpio_and_timer(void) {
//    // Enable GPIOB clock
//    SYSCTL->RCGCGPIO |= 0x02;
//    while ((SYSCTL->PRGPIO & 0x02) == 0); // Wait for GPIOB clock to stabilize

//    // Configure PB6 as output for TRIG signal
//    GPIOB->DIR |= (1 << TRIG_PIN);  // Set PB6 as output
//    GPIOB->DEN |= (1 << TRIG_PIN);  // Enable digital functionality on PB6
//    GPIOB->AMSEL &= ~(1 << TRIG_PIN); // Disable analog functionality on PB6

//    // Configure PB4 as input with alternate function (T1CCP0)
//    GPIOB->DIR &= ~(1 << ECHO_PIN);     // PB4 as input
//    GPIOB->AFSEL |= (1 << ECHO_PIN);    // Enable alternate function on PB4
//    GPIOB->PCTL = (GPIOB->PCTL & 0xFFF0FFFF) | 0x00070000; // Set PB4 for T1CCP0
//    GPIOB->DEN |= (1 << ECHO_PIN);      // Enable digital functionality on PB4
//    GPIOB->AMSEL &= ~(1 << ECHO_PIN);   // Disable analog functionality on PB4

//    // Add a delay to avoid transient voltage issues
//    for (volatile int i = 0; i < 1600000; i++);

//    // Enable Timer1 clock
//    SYSCTL->RCGCTIMER |= 0x02;
//    while ((SYSCTL->PRTIMER & 0x02) == 0); // Wait for Timer1 clock to stabilize

//    // Configure TIMER1A for edge-time mode
//    TIMER1->CTL &= ~0x01;      // Disable TIMER1A during setup
//    TIMER1->CFG = 0x04;        // Set 16-bit timer mode
//    TIMER1->TAMR = 0x17;       // Edge-time mode, capture both edges
//    TIMER1->CTL |= 0x0C;       // Capture both rising and falling edges
//    TIMER1->IMR &= ~0x03;      // Disable interrupts (polling mode)
//    TIMER1->CTL |= 0x01;       // Enable TIMER1A
//}

void send_trigger_pulse(void) {
    // Ensure TRIG is LOW
    GPIOB->DATA &= ~(1 << TRIG_PIN);
    for (volatile int i = 0; i < 1000; i++); // Short delay (~2 us)

    // Send HIGH pulse (>10us)
    GPIOB->DATA |= (1 << TRIG_PIN);
    for (volatile int i = 0; i < 1000; i++); // Delay (~10 us)

    // Set TRIG back to LOW
    GPIOB->DATA &= ~(1 << TRIG_PIN);
}

int dist_sense(int* dist_arr) {
		uint32_t rise_time, fall_time, pulse_width;
		// Send trigger pulse
		send_trigger_pulse();
	
		// Wait for rising edge (CAERIS flag)
		while ((TIMER1->RIS >> 2 &1) == 0);
		rise_time = TIMER1->TAR;  // Capture rising edge time
		TIMER1->ICR = 0x04;       // Clear capture flag or  TIMER1->ICR |= (1 << 2); 

		// Wait for falling edge (CAERIS flag)
		while ((TIMER1->RIS >> 2 &1) == 0);
		fall_time = TIMER1->TAR;  // Capture falling edge time
		TIMER1->ICR = 0x04;       // Clear capture flag

		// Measure pulse width
		pulse_width = (fall_time - rise_time) / 16; // Convert to microseconds 1/16000000

		// Calculate distance and write to array
		*dist_arr = (pulse_width * 0.34) /2 /10;     // in cm record to array
		
		return 1;
}

