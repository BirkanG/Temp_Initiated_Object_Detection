#include "Nokia5110.h"
#include <stdbool.h> // For boolean type
#include <stdint.h>

void print_number_int(uint8_t x, uint8_t y, int number) {
    char buffer[12]; // Buffer to hold the ASCII representation of the number
    int index = 0;

    // Handle negative numbers
    if (number < 0) {
        buffer[index++] = '-'; // Add minus sign for negative numbers
        number = -number;      // Convert number to positive
    }

    // Convert number to ASCII digits (store in reverse order)
    do {
        buffer[index++] = (number % 10) + '0'; // Extract the last digit and convert to ASCII
        number /= 10;                          // Remove the last digit
    } while (number > 0);

    // Null-terminate the string
    buffer[index] = '\0';
		
    // Reverse the buffer to get the correct order
    if (buffer[0] == '-') { // Handle negative numbers
        for (int i = 1; i < index / 2 + 1; i++) {
            char temp = buffer[i];
            buffer[i] = buffer[index - 1 - (i - 1)];
            buffer[index - 1 - (i - 1)] = temp;
        }
    } else { // For positive numbers
        for (int i = 0; i < index / 2; i++) {
            char temp = buffer[i];
            buffer[i] = buffer[index - 1 - i];
            buffer[index - 1 - i] = temp;
        }
    }

    // Set cursor position on the LCD
    Nokia5110_SetCursor(x, y);

    // Print the number string on the LCD
    Nokia5110_OutString(buffer);
}

void print_number_float(uint8_t x, uint8_t y, float number) {
    char buffer[16]; // Buffer to hold the ASCII representation of the number
    int index = 0;

    // Handle negative numbers
    if (number < 0) {
        buffer[index++] = '-'; // Add minus sign for negative numbers
        number = -number;      // Convert number to positive
    }

    // Separate integer and fractional parts
    int integerPart = (int)number;           // Get the integer part
    float fractionalPart = number - integerPart; // Get the fractional part

    // Convert the integer part to ASCII digits (store in reverse order)
    do {
        buffer[index++] = (integerPart % 10) + '0'; // Extract the last digit and convert to ASCII
        integerPart /= 10;                          // Remove the last digit
    } while (integerPart > 0);

    // Null-terminate and reverse the integer part
    int start = (buffer[0] == '-') ? 1 : 0; // Skip '-' when reversing
    int end = index - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start++] = buffer[end];
        buffer[end--] = temp;
    }

    // Add the decimal point
    buffer[index++] = '.';

    // Convert the fractional part to 2 decimal places
    fractionalPart *= 100; // Shift two decimal places
    int fractionalInt = (int)(fractionalPart + 0.5); // Round to the nearest integer
    buffer[index++] = (fractionalInt / 10) + '0';    // Extract the tens place
    buffer[index++] = (fractionalInt % 10) + '0';    // Extract the ones place

    // Null-terminate the string
    buffer[index] = '\0';

    // Set cursor position on the LCD
    Nokia5110_SetCursor(x, y);

    // Print the formatted string on the LCD
    Nokia5110_OutString(buffer);
}
