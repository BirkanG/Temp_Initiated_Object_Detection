#include "TM4C123GH6PM.h"
#include "scan_object.h"
#include "Nokia5110.h"
#include "bmp280.h"
//#include "lm35_V3.h"

void init_GPIO(void) {
    SYSCTL->RCGCGPIO |= 0x22;    // Enable clock for GPIOB and GPIOF
    while ((SYSCTL->PRGPIO & 0x02) == 0); // Wait for GPIOB clock to stabilize
		while ((SYSCTL->PRGPIO & 0x20) == 0); // Wait for GPIOF clock to stabilize

		//--For led pins----------
		GPIOF->DIR |= 0xE;           // Set PF1-PF3 for led indicators on board
		GPIOF->DEN |= 0xE;
		//------------------------

		//--For motor drive pins--
		GPIOB->DIR |= 0x0F;      	   // Set PB0-PB3 as outputs for step motor
    GPIOB->DEN |= 0x0F;          // Enable digital function for PB0-PB3
		//------------------------

		//--For dist_sense pins---
    // Configure PB5 as output for TRIG signal
    GPIOB->DIR |= (1 << TRIG_PIN);  // Set trig pin as output
    GPIOB->DEN |= (1 << TRIG_PIN);  // Enable digital functionality on trig pin
    GPIOB->AMSEL &= ~(1 << TRIG_PIN); // Disable analog functionality on trig pin

    // Configure PB4 as input with alternate function (T1CCP0)
    GPIOB->DIR &= ~(1 << ECHO_PIN);     // echo pin as input
    GPIOB->AFSEL |= (1 << ECHO_PIN);    // Enable alternate function on echo pin
    GPIOB->PCTL = (GPIOB->PCTL & 0xFFF0FFFF) | 0x00070000; // Set echo pin for T1CCP0
    GPIOB->DEN |= (1 << ECHO_PIN);      // Enable digital functionality on echo pin
    GPIOB->AMSEL &= ~(1 << ECHO_PIN);   // Disable analog functionality on echo pin
		//--------------------------

		//--For keypad button pins--
    SYSCTL->RCGCGPIO |= 0x08; // GPIOD
    while ((SYSCTL->PRGPIO & 0x08) == 0); // Wait for GPIOD clock to stabilize

    // Configure PD0-PD3 as inputs with pull-up resistors
    GPIOD->DIR &= ~0x0F;      // PD0-PD3 as inputs
    GPIOD->DEN |= 0x0F;       // Enable digital function for PD0-PD3
    GPIOD->PUR |= 0x0F;       // Enable pull-up resistors

    // Configure falling-edge interrupt for PD0-PD3
    GPIOD->IS &= ~0x0F;       // Edge-sensitive
    GPIOD->IBE &= ~0x0F;      // Single edge
    GPIOD->IEV &= ~0x0F;      // Falling edge
    GPIOD->IM |= 0x0F;        // Enable interrupt for PD0-PD3

    // Enable GPIOD interrupt in NVIC
    NVIC->ISER[0] |= (1 << 3); // Enable IRQ3 for GPIOD
		//-------------------------

    // Add a delay to avoid transient voltage issues
    for (volatile int i = 0; i < 1600000; i++);

    // Enable Timer1 clock
    SYSCTL->RCGCTIMER |= 0x02;
    while ((SYSCTL->PRTIMER & 0x02) == 0); // Wait for Timer1 clock to stabilize

    // Configure TIMER1A for edge-time mode
    TIMER1->CTL &= ~0x01;      // Disable TIMER1A during setup
    TIMER1->CFG = 0x04;        // Set 16-bit timer mode
    TIMER1->TAMR = 0x17;       // Edge-time mode, capture both edges
    TIMER1->CTL |= 0x0C;       // Capture both rising and falling edges
    TIMER1->IMR &= ~0x03;      // Disable interrupts (polling mode)
    TIMER1->CTL |= 0x01;       // Enable TIMER1A
		//------------------------
}

void init_Comparator0(void) {
    // Enable clock for GPIOC and Comparator module
    SYSCTL->RCGCGPIO |= (1 << 2);  // Enable clock for Port C
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
    //COMP->ACINTEN |= (1 << 0);          // Enable interrupts for Comparator 0 at start we dont want to interrupt. just enable it before deep sleep
		NVIC_EnableIRQ(COMP0_IRQn); // Enable in NVIC
}

void COMP0_Handler(void) {
    COMP->ACMIS = 0x01;  // Clear interrupt flag
    for (int i = 0; i < 100000; i++);  // Delay loop
    if ((COMP->ACSTAT0 & 0x02) == 0x02) {  // Check comparator output
        COMP->ACINTEN &= ~0x1;  // Disable comparator interrupt
        in_deep_sleep = 0;
				state = 3;              // analog value exceeds go awake state (3)
    }
}

void init_SysTick(void) {
    SysTick->LOAD = 16000000 - 1;  // Load value for 1-second interval (16 MHz clock)
    SysTick->VAL = 0;             // Clear current value
    SysTick->CTRL = 0x05;         // disabled SysTick with system clock with 16mhz set and interrupts set. 
																	// it will be enabled on case 3 (when we want to update the digital temp)
		//NVIC_SetPriority(SysTick_IRQn,2);
}

//void SysTick_Handler(void) {
//    // Update digital temperature
//    digital_temp = BMP280_GetAverageTemperature();
//}

int main(void) {	
    Nokia5110_Init();
    configureDeepSleep();   // Configure deep sleep settings
    init_GPIO();            // Initialize GPIO for motor, LEDs, HCSR, PD pins for keypad
    init_Comparator0();     // Initialize comparator0
    I2C2_Init();        // Initialize I2C2
    BMP280_Init();      // Initialize BMP280
		LED_init();
		init_SysTick();
		ADC1_Init();
		Nokia5110_Clear();
		//move_motor(10);
		
		state = 1;							// Start in deep sleep
		int distance_array_size = 21;
    int distance_array[distance_array_size];  // Total 19 degrees (-90 to 90). Last two are for min dist and angle
    int* dist_array_ptr;
		analog_temp_thr = (ADC1_Read() * 3.3) / 4096.0 * 100;
		float digital_temp = BMP280_GetAverageTemperature();
	
    while (1) {	
			switch (state) {
				case 1:          // in deep sleep
					// clear array and LCD screen
					Nokia5110_Clear();
					for (int i = 0; i < distance_array_size; i++) {
						distance_array[i] = 0;
					}
					Nokia5110_SetCursor(0, 0);
					Nokia5110_OutString("Sleep   Mode");
					Nokia5110_SetCursor(0, 2);
					Nokia5110_OutString("Digital");
					Nokia5110_SetCursor(0, 3);
					Nokia5110_OutString("Threshold:");
					print_number_int(10, 3, digital_temp_thr);
					Nokia5110_SetCursor(0,4);
					Nokia5110_OutString("Analog");
					Nokia5110_SetCursor(0,5);
					Nokia5110_OutString("Thr:");
					print_number_float(4,5,analog_temp_thr);
					enterDeepSleep();
					break;

				case 2:              // in scan object state
					// Clear the distance array before scanning
					for (int i = 0; i < distance_array_size; i++) {
						distance_array[i] = 0;
					}
					
					Nokia5110_Clear();
					dist_array_ptr = distance_array;
					scan_object(dist_array_ptr);
					Delay(1);
					if (in_deep_sleep == 1) {
						state = 1;
						scanned_once = 0;
					}
					else {
						state = 3;
						scanned_once = 1;
					}
					break;
					
				case 3:               // system awake state
					// Check if 1 second has elapsed
					turn_on_LED();
					uint32_t current_time = SysTick->VAL;
					static uint32_t last_time_update = 0;
					if ( ((last_time_update - current_time) & 0xFFFFFF) >= 15000000) {  // ~1 second
							digital_temp = BMP280_GetAverageTemperature();  // Update temperature
							last_time_update = current_time;  // Reset last update time
					}
					Nokia5110_Clear();
				
					if (page%3 == 1) {
							// Count non-zero elements in the distance array.
							int num_points = 0;
							for (int i = 0; i < (distance_array_size-2); i++) {
									if (distance_array[i] != 0) {
											num_points++;
									}
									if (num_points == (distance_array_size-2)) break;  // Max array size for distance values. cpooment out
							}

							PlotScanData(dist_array_ptr, num_points);			
					} else if (page%3 == 0){
							Nokia5110_SetCursor(3,0);
							Nokia5110_OutString("Object");
							Nokia5110_SetCursor(0,2);
							Nokia5110_OutString("Dist:");
							print_number_int(5,2,distance_array[19]);
							Nokia5110_SetCursor(0,4);
							Nokia5110_OutString("Angle:");
							print_number_int(6,4,distance_array[20]);
					} else {
							Nokia5110_SetCursor(0,0);
							Nokia5110_OutString("Digital");
							Nokia5110_SetCursor(0,1);
							Nokia5110_OutString("Threshold:");
							print_number_int(10,1,digital_temp_thr);
							Nokia5110_SetCursor(0,3);
							Nokia5110_OutString("Digital");
							Nokia5110_SetCursor(0,4);
							Nokia5110_OutString("Temp:");
							print_number_float(5,4,digital_temp);
					}
					if (digital_temp > digital_temp_thr && !scanned_once) {
						state = 2;
					}
					Delay(10);  //wait,  refresh data etc. (for now push s1 to go scan mode)
					break;
			}
		}
    return 0;
}

