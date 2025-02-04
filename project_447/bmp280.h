#include "TM4C123GH6PM.h"
#include <stdint.h>
#include <stdio.h>

//extern void OutStr(const char *str);

#define BMP280_I2C_ADDR 0x76

// Calibration data
uint16_t DIG_T1;
int16_t DIG_T2, DIG_T3;

// I2C2 Initialization
void I2C2_Init(void) {
    SYSCTL->RCGCI2C |= (1 << 2);  // Enable clock for I2C2
    SYSCTL->RCGCGPIO |= (1 << 4); // Enable clock for GPIO Port E
    __ASM("NOP"); __ASM("NOP"); __ASM("NOP");
    while (!(SYSCTL->PRGPIO & (1 << 4))); // Wait for GPIO Port E to be ready

    GPIOE->AFSEL |= 0x30;  // Enable alternate functions on PE4 (SCL) and PE5 (SDA)
    GPIOE->ODR |= 0x20;    // Enable open-drain on PE5 (SDA)
    GPIOE->DEN |= 0x30;    // Enable digital function on PE4 and PE5
    GPIOE->PCTL = (GPIOE->PCTL & 0xFF00FFFF) | 0x00330000; // Assign I2C to PE4 and PE5
    GPIOE->PUR |= 0x30;    // Enable pull-up for PE4 (SCL) and PE5 (SDA)

    I2C2->MCR = 0x10;   // Initialize as master
    I2C2->MTPR = 7;     // Set clock speed to 100 kHz (assuming 16 MHz system clock)
}

// I2C Write Function
void I2C_Write(uint8_t slave_addr, uint8_t reg_addr, uint8_t data) {
    I2C2->MSA = (slave_addr << 1) | 0x00; // Set slave address and indicate write
    I2C2->MDR = reg_addr;                 // Set register address
    I2C2->MCS = 0x03;                     // Send START + address + write
    while (I2C2->MCS & 0x01);             // Wait for transmission
    I2C2->MDR = data;                     // Send data
    I2C2->MCS = 0x05;                     // Send data + STOP
    while (I2C2->MCS & 0x01);             // Wait for completion
}

// I2C Read Multiple Bytes
void I2C_Read_Multiple(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        // Step 1: Send the slave address with a write bit (0) and the register address
        I2C2->MSA = (slave_addr << 1) | 0x00; // Write mode
        I2C2->MDR = reg_addr + i;            // Set register address (incremented manually)
        I2C2->MCS = 0x03;                    // START + RUN
        while (I2C2->MCS & 0x01);            // Wait while BUSY
        if (I2C2->MCS & 0x02) {              // Check for error
            I2C2->MCS = 0x04;                // Send STOP
            return;
        }

        // Step 2: Read data from the slave
        I2C2->MSA = (slave_addr << 1) | 0x01; // Read mode
        I2C2->MCS = 0x07;                     // START + RUN + STOP
        while (I2C2->MCS & 0x01);             // Wait while BUSY
        if (I2C2->MCS & 0x02) {               // Check for error
            I2C2->MCS = 0x04;                 // Send STOP
            return;
        }
        data[i] = I2C2->MDR;                  // Read data byte
    }
}

// BMP280 Read Calibration Data
void BMP280_ReadCalibration(void) {
    uint8_t calib[6];
    I2C_Read_Multiple(BMP280_I2C_ADDR, 0x88, calib, 6);

    DIG_T1 = (uint16_t)(calib[1] << 8 | calib[0]);
    DIG_T2 = (int16_t)(calib[3] << 8 | calib[2]);
    DIG_T3 = (int16_t)(calib[5] << 8 | calib[4]);
}

// BMP280 Initialization
void BMP280_Init(void) {
    uint8_t ctrl_meas, config;
    I2C_Write(0x76, 0xF4, 0x27);  // Write to control measurement register (Normal mode, Temp+Pressure oversampling)
    I2C_Read_Multiple(0x76, 0xF4, &ctrl_meas, 1);
    I2C_Write(0x76, 0xF5, 0xA0); // Write to config register (standby time, filter)
    I2C_Read_Multiple(0x76, 0xF5, &config, 1);
    char buffer[50];
    sprintf(buffer, "Ctrl: 0x%02X, Config: 0x%02X\n \r\4", ctrl_meas, config);
    //OutStr(buffer);
    BMP280_ReadCalibration();
}

// BMP280 Test
void BMP280_Test(void) {
    uint8_t chip_id;
    I2C_Read_Multiple(BMP280_I2C_ADDR, 0xD0, &chip_id, 1);
    char buffer[50];
    sprintf(buffer, "BMP280 detected, Chip ID: 0x%02X\n \r\4", chip_id);
    //OutStr(buffer);
}

// BMP280 Read Temperature
uint32_t BMP280_ReadTemperature(void) {
    uint8_t data[3];
    long raw_temp, var1, var2, t_fine, T;

    // Read raw temperature data
    I2C_Read_Multiple(0x76, 0xFA, data, 3);
    raw_temp = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | (data[2] >> 4);

    // Apply compensation formula
    var1 = (((raw_temp >> 3) - ((int32_t)DIG_T1 << 1)) * ((int32_t)DIG_T2)) >> 11;
    var2 = (((((raw_temp >> 4) - ((int32_t)DIG_T1)) * ((raw_temp >> 4) - ((int32_t)DIG_T1))) >> 12) *
            ((int32_t)DIG_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8; // Temperature in 0.01 °C

    return T; // Return temperature in 0.01 °C
}

// Compute Average Temperature
float BMP280_GetAverageTemperature(void) {
    uint32_t sum = 0;
    for (int i = 0; i < 128; i++) {
        sum += BMP280_ReadTemperature();
    }
    return sum / 128.0f / 100.0f; // Convert to °C
}

//// Main Function
//int main(void) {
//    char buffer[50];

//    I2C2_Init();        // Initialize I2C2
//    BMP280_Init();      // Initialize BMP280
//    BMP280_Test();      // Test BMP280 communication

//    while (1) {
//        float temp = BMP280_GetAverageTemperature();
//        sprintf(buffer, "Average Temperature: %.2f C\n \r\4", temp);
//        OutStr(buffer);  // Print average temperature
//        for (volatile int i = 0; i < 1000000; i++); // Delay
//    }
//}


