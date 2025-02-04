#include "TM4C123GH6PM.h" // Required for register addresses

int step =0;
float step_dest=0;

//void init_GPIO(void) {
//    SYSCTL->RCGCGPIO |= 0x22;    // Enable clock for GPIOB and GPIOF
//    while ((SYSCTL->PRGPIO & 0x02) == 0); // Wait for GPIOB clock to stabilize
//		while ((SYSCTL->PRGPIO & 0x20) == 0); // Wait for GPIOF clock to stabilize
//		
//		//--For led pins----------
//		GPIOF->DIR |= 0xE;           // Set PF1-PF3 for led indicators on board
//		GPIOF->DEN |= 0xE;
//		//------------------------
//	
//	
//		//--For motor drive pins--
//		GPIOB->DIR |= 0x0F;      	   // Set PB0-PB3 as outputs for step motor
//    GPIOB->DEN |= 0x0F;          // Enable digital function for PB0-PB3
//		//------------------------
//	
//		//--For dist_sense pins---
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
//		//------------------------
//}

// Function to Set Step Signals
void set_step(unsigned int step_no) {
    // Ensure step_no is between 0 and 7
		step_no = step_no % 8; 

    // Step sequence table for specific pins
    // [PB3, PB2, PB1, PB0]
    const uint8_t step_table[8][4] = {
        {1, 0, 0, 0}, // Step 1: PB3 = 1
        {1, 1, 0, 0}, // Step 2: PB3,2 = 1
        {0, 1, 0, 0}, // Step 3: PB2 = 1
        {0, 1, 1, 0}, // Step 4: PB2,1 = 1
        {0, 0, 1, 0}, // Step 5: PB1 = 1
        {0, 0, 1, 1}, // Step 6: PB1,0 = 1
        {0, 0, 0, 1}, // Step 7: PB0 = 1
        {1, 0, 0, 1}  // Step 8: PB0,3 = 1
		};

    // Update PB3
    GPIOB->DATA = (GPIOB->DATA & ~((1 << 3) )) | 
                  (step_table[step_no][0] << 3);

    // Update PB2
    GPIOB->DATA = (GPIOB->DATA & ~(1 << 2)) | 
                  (step_table[step_no][1] << 2);

    // Update PB1
    GPIOB->DATA = (GPIOB->DATA & ~(1 << 1)) | 
                  (step_table[step_no][2] << 1);
		
		// Update PB0
    GPIOB->DATA = (GPIOB->DATA & ~(1 << 0)) | 
                  (step_table[step_no][3] << 0);
}

void Delay(unsigned int time) {
    volatile unsigned int i;
    for (i = 0; i < time * 4000; i++); // Approx 0.1 second delay for Delay(25)
}

void low_Delay(unsigned int time) {
    volatile unsigned int i;
    for (i = 0; i < time ; i++);
}

void move_motor(float degree) {
		step_dest += degree * 2*2048 / 360;    //since half step is used 2*2048. full step ->1*2048
		while ((int)step_dest > step){
			step += 1;
			set_step(step);
			low_Delay(2000);
		}
		while ((int)step_dest < step){
			step -= 1;
			set_step(step);
			low_Delay(2000);
		}
}

//int main(void) {
//		int distance_array[20];
//		int* dist_array_ptr;
//    init_GPIO();    // Initialize GPIO for motor and buttons
//		//move_motor(-5);
//		while(1) {
//			dist_array_ptr = distance_array;

//			for (int k=0; k<=180; k = k+10) {
//				dist_array_ptr += dist_sense(dist_array_ptr);
//				if (*(dist_array_ptr-1) <= 15) {   			// red
//					GPIOF->DATA &= ~(1 << 2);
//					GPIOF->DATA &= ~(1 << 3);
//					GPIOF->DATA |= (1 << 1);
//				}
//				else if (*(dist_array_ptr-1) <= 30) {   // blue
//					GPIOF->DATA &= ~(1 << 1);
//					GPIOF->DATA &= ~(1 << 3);
//					GPIOF->DATA |= (1 << 2);
//				}
//				else {   													  // green
//					GPIOF->DATA &= ~(1 << 1);
//					GPIOF->DATA &= ~(1 << 2);
//					GPIOF->DATA |= (1 << 3);
//				}
//				move_motor(10);
//				Delay(1);  // comment out?
//			}
//			GPIOF->DATA &= ~(1 << 1);
//			GPIOF->DATA &= ~(1 << 2);
//			GPIOF->DATA &= ~(1 << 3);			

//			Delay(500);
//			move_motor(-190);
//			Delay(500);
//			
//			
//			// adjusting motor (comment out)
////			move_motor(180);
////			Delay(500);
////			move_motor(-180);
////			Delay(500);
//		}
//		
//    return 0;
//}
