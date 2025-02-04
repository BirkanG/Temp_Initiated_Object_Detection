#include "motor_move.h"
#include "dist_sense.h"
#include "Nokia5110.h"
#include "print_number.h"
#include "deep_sleep.h"
#include <math.h>

#define OFFSET_X (MAX_X / 2)  // Center of the screen for x-axis
#define OFFSET_Y (MAX_Y - 2) // Bottom of the screen for y-axis
#define MAX_SENSOR_DISTANCE 100  // Maximum distance in cm

//if you change the angle or step, do not forget to update the distance_array size accordingly in the main file

void PlotScanData(int* distances, int numPoints) {
    Nokia5110_ClearBuffer();  // Clear the buffer before plotting

    // Draw the scanner location as a reversed triangle (bottom middle of the screen)
    int centerX = OFFSET_X;
    int centerY = OFFSET_Y + 1;
    Nokia5110_SetPxl(centerY, centerX - 2); // Left extended base
    Nokia5110_SetPxl(centerY, centerX - 1); // Left base
    Nokia5110_SetPxl(centerY, centerX);     // Center base
    Nokia5110_SetPxl(centerY, centerX + 1); // Right base
    Nokia5110_SetPxl(centerY, centerX + 2); // Right extended base
    Nokia5110_SetPxl(centerY - 1, centerX - 1); // Upper left
    Nokia5110_SetPxl(centerY - 1, centerX);     // Upper middle
    Nokia5110_SetPxl(centerY - 1, centerX + 1); // Upper right
    Nokia5110_SetPxl(centerY - 2, centerX);     // Tip of the triangle

    // Plot the scanned data as 2x2 blocks
    for (int i = 0; i < numPoints; i++) {
        int distance = distances[i];
        int angle = 0 + i * 10;  // Infer angle from the index

        // Clip and normalize distance
        if (distance > MAX_SENSOR_DISTANCE) {
            distance = MAX_SENSOR_DISTANCE;
        }
        float scaledDistance = ((float)distance * (float)OFFSET_Y) / (float)MAX_SENSOR_DISTANCE;

        // Convert polar to Cartesian based on sensor location
        float radian = angle * (3.14159 / 180);
        int x = OFFSET_X + (int)(scaledDistance * cos(radian));
        int y = OFFSET_Y - (int)(scaledDistance * sin(radian));

        // Adjust coordinates to stay within screen bounds
        if (x < 0) x = 0;
        if (x >= MAX_X) x = MAX_X - 2;
        if (y < 0) y = 0;
        if (y >= MAX_Y) y = MAX_Y - 2;

        // Draw a 2x2 block for better visibility
        for (int dx = 0; dx < 2; dx++) {
            for (int dy = 0; dy < 2; dy++) {
                int px = x + dx;
                int py = y + dy;
                // Ensure block coordinates are still valid
                if (px >= 0 && px <= MAX_X && py >= 0 && py <= MAX_Y) {
                    Nokia5110_SetPxl(py, px);
                }
            }
        }
    }

    Nokia5110_DisplayBuffer();  // Update the LCD with the new buffer
}




void scan_object(int* dist_array_ptr) {
		//clear leds
		GPIOF->DATA &= ~(1 << 1);
		GPIOF->DATA &= ~(1 << 2);
		GPIOF->DATA &= ~(1 << 3);
		
		// move motor to starting point
		move_motor(-90);
		Delay(250);

		// print no object detected
		Nokia5110_Clear();
		Nokia5110_SetCursor(2,0);
		Nokia5110_OutString("NO OBJECT");
		Nokia5110_SetCursor(2,1);
		Nokia5110_OutString("DETECTED!");
	
    int min_distance = 100; // Initialize to a value larger than any expected distance
    int min_angle = -90;
		int angle_return;

    for (int k = -90; k <= 90; k += 10) {
				angle_return= k * (-1);
				if (in_deep_sleep) {
					break;
				}
				dist_array_ptr += dist_sense(dist_array_ptr);
        int current_distance = *(dist_array_ptr-1);

        if (current_distance < min_distance) {
            min_distance = current_distance;
            min_angle = k;
					
						Nokia5110_Clear();
						Nokia5110_SetCursor(3,0);
						Nokia5110_OutString("Object");
						Nokia5110_SetCursor(0,2);
						Nokia5110_OutString("Dist:");
						print_number_int(5,2,min_distance);
						Nokia5110_SetCursor(0,4);
						Nokia5110_OutString("Angle:");
						print_number_int(6,4,min_angle);
        }

        if (current_distance <= 50) { // Red
            GPIOF->DATA &= ~(1 << 2);
            GPIOF->DATA &= ~(1 << 3);
            GPIOF->DATA |= (1 << 1);
        } else if (current_distance <= 75) { // Blue
            GPIOF->DATA &= ~(1 << 1);
            GPIOF->DATA &= ~(1 << 3);
            GPIOF->DATA |= (1 << 2);
        } else if (current_distance <= 100) { // Green
            GPIOF->DATA &= ~(1 << 1);
            GPIOF->DATA &= ~(1 << 2);
            GPIOF->DATA |= (1 << 3);
        } else { // No object detected
            GPIOF->DATA &= ~(1 << 1);
            GPIOF->DATA &= ~(1 << 2);
            GPIOF->DATA &= ~(1 << 3);
        }

        move_motor(10);
        //Delay(1); 
    }
		
    // Append minimum distance and angle to end of the array
    *dist_array_ptr = min_distance;
    dist_array_ptr++;
    *dist_array_ptr = min_angle;

    // Keep the corresponding LED for the minimum distance on
    if ( (min_distance <= 50) & (in_deep_sleep==0)) { // Red
        GPIOF->DATA &= ~(1 << 2);
        GPIOF->DATA &= ~(1 << 3);
        GPIOF->DATA |= (1 << 1);
    } else if ( (min_distance <= 75) & (in_deep_sleep==0)) { // Blue
        GPIOF->DATA &= ~(1 << 1);
        GPIOF->DATA &= ~(1 << 3);
        GPIOF->DATA |= (1 << 2);
    } else if ( (min_distance <= 100) & (in_deep_sleep==0) ) { // Green
        GPIOF->DATA &= ~(1 << 1);
        GPIOF->DATA &= ~(1 << 2);
        GPIOF->DATA |= (1 << 3);
    } else { // No object detected
        GPIOF->DATA &= ~(1 << 1);
        GPIOF->DATA &= ~(1 << 2);
        GPIOF->DATA &= ~(1 << 3);
    }
		
    //Delay(1); // Wait 
		if (in_deep_sleep) {
			Nokia5110_Clear();
			move_motor(angle_return);
		}
		else {
			move_motor(-100);
		}
    Delay(500);
}
