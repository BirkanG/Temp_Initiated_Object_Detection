#include "TM4C123GH6PM.h"
#include "lm35_V3.h"
#include "init_speaker.h"
#include "LED.h"

#define SYSCTL_DSLPPWRCFG_SRAMPM_NRM  0x00000003
#define SYSCTL_DSLPPWRCFG_FLASHPM_NRM  0x00000030
#define SYSCTL_DSLPCLOCKCFG_D_MOSC  0x00000000

extern void a_delay(int delay); 
extern void GPIOD_Handler(void); // Declared as external, implemented in assembly

volatile int in_deep_sleep = 1; // Flag to indicate deep sleep state (starts in deep sleep)
volatile int state = 1;          // states (1, deep sleep) (2, object scan) (3, awake)
volatile int page = 0;
volatile int scanned_once = 0;
volatile int analog_temp_thr_adc_value;

void GPIOF_Handler(void) {
		a_delay(400000);
	
		if ((GPIOF->DATA & (1<<4)) == 0) {
			in_deep_sleep = 1;
			state = 1;
			turn_off_LED();
			GPIOB->DATA &= ~0x0F;   // turn off motor leds
			scanned_once = 0;    // clean object scanned flag
		}
    // Clear the interrupt flag for PF4	
    GPIOF->ICR |= (1 << 4); // Clear interrupt
}

//void GPIOD_Handler(void) {
//    a_delay(400000); // Debounce delay

//    // Check if PD0 triggered the interrupt
//    if (GPIOD->MIS & (1 << 0)) {
//        if (!(GPIOD->DATA & (1 << 0)) && (digital_temp_thr > min_digital_temp_thr)) {  
//            digital_temp_thr -= 1; // Decrease threshold
//        }
//        GPIOD->ICR |= (1 << 0); // Clear interrupt for PD0
//    }

//    // Check if PD1 triggered the interrupt
//    if (GPIOD->MIS & (1 << 1)) {
//        if (!(GPIOD->DATA & (1 << 1)) && (digital_temp_thr >= min_digital_temp_thr)) {  
//            digital_temp_thr += 1; // Increase threshold
//        }
//        GPIOD->ICR |= (1 << 1); // Clear interrupt for PD1
//    }
//		
//		if (GPIOD->MIS & (1 << 2)) {
//				if (!(GPIOD->DATA & (1 << 2))) {  
//						page = (page+1) % 100; // change page
//				}
//				GPIOD->ICR |= (1 << 2); // Clear interrupt for PD2
//		}
//		
//		if (GPIOD->MIS & (1 << 3)) {          // refresh threshold
//				if (!(GPIOD->DATA & (1 << 3))) { 
//						analog_temp_thr = (ADC1_Read() * 3.3) / 4096.0 * 100;
//						a_delay(100000);
//						
//				}
//				GPIOD->ICR |= (1 << 3); // Clear interrupt for PD3
//		}
//}

void configureWakeSources(void) {
    // Enable clock for GPIOF (used for wake-up button)
    SYSCTL->RCGCGPIO |= (1 << 5); // GPIOF
    while ((SYSCTL->PRGPIO & (1 << 5)) == 0); // Wait until GPIOF is ready

    // Configure PF4 as input with pull-up resistor
    GPIOF->DIR &= ~(1 << 4);  // PF4 as input
    GPIOF->DEN |= (1 << 4);   // Enable digital functionality for PF4
    GPIOF->PUR |= (1 << 4);   // Enable pull-up for PF4

    // Configure PF4 for falling-edge interrupt
    GPIOF->IS &= ~(1 << 4);   // Edge-sensitive
    GPIOF->IBE &= ~(1 << 4);  // Single edge
    GPIOF->IEV &= ~(1 << 4);  // Falling edge
    GPIOF->IM |= (1 << 4);    // Enable interrupt for PF4

    // Enable GPIOF interrupt in NVIC
    NVIC->ISER[0] |= (1 << 30); // Enable IRQ30 for GPIOF
}

void configureDeepSleep(void) {
    // Configure peripherals to remain powered during deep sleep
    SYSCTL->DSLPPWRCFG |= (SYSCTL_DSLPPWRCFG_SRAMPM_NRM | SYSCTL_DSLPPWRCFG_FLASHPM_NRM);

    // Enable GPIOA and SSI0 clocks during deep sleep  (necessary for comparator pins)
    //SYSCTL->SCGCGPIO |= (1 << 0); // GPIOA
    //SYSCTL->SCGCSSI |= (1 << 0);  // SSI0
	
		configureWakeSources();
	
    // Enable deep sleep mode in SCB
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
}

void enterDeepSleep(void) {
		//in_deep_sleep = 0;
		GPIOB->DATA &= ~0x0F;   // turn off motor leds
		GPIOF->DATA &= ~(1 << 1);
    GPIOF->DATA &= ~(1 << 2);
    GPIOF->DATA &= ~(1 << 3);
		COMP->ACINTEN |= (1 << 0);          // Enable interrupts for Comparator 0, in deep sleep
		a_delay(20000);
    __asm(" WFI"); // Wait For Interrupt to enter deep sleep
}
