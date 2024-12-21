#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

// Default I2C address for ADS1115 (ADDR connected to GND)
#define ADS1115_I2C_ADDRESS 0x48

// Function to configure ADS1115
// This function configures the ADS1115 for single-ended mode, AIN0, PGA = +/-4.096V, DR = 1600SPS, and single-shot mode
// Adjust the configuration register settings as needed
static void configure_ads1115(int fd, int channel) {
    // Config register (0x01) - Write 2 bytes:
    // MSB: 0xC3 (Single-shot mode, AIN0, ±4.096V)
    // LSB: 0x83 (1600SPS, single-shot mode)
    // Change values if needed for different ranges, sampling rates, or channels
    unsigned char config[3];
    config[0] = 0x01; // Config register address
    
    // Default to AIN0 (channel 0)
    unsigned char channel_config = 0xC3;

    // Select the correct channel
    switch(channel) {
        case 0: channel_config = 0xC3; break; // AIN0
        case 1: channel_config = 0xD3; break; // AIN1
        case 2: channel_config = 0xE3; break; // AIN2
        case 3: channel_config = 0xF3; break; // AIN3
        default: channel_config = 0xC3;        // Default to AIN0
    }

    config[1] = channel_config;
    config[2] = 0x83; // Single-shot mode, 1600SPS

    // Write configuration to ADS1115
    if (write(fd, config, 3) != 3) {
        perror("Failed to write config to ADS1115");
        exit(1);
    }

    // Wait for conversion to complete (about a few milliseconds)
    usleep(100000); 
}

// Function to read the converted value from ADS1115
static int read_ads1115(int fd) {
    unsigned char reg[1] = {0x00}; // Conversion register address
    if (write(fd, reg, 1) != 1) {
        perror("Failed to set read register");
        exit(1);
    }

    // Read 2 bytes of data
    unsigned char data[2];
    if (read(fd, data, 2) != 2) {
        perror("Failed to read data from ADS1115");
        exit(1);
    }

    // Combine bytes into a signed 16-bit integer
    int value = (data[0] << 8) | data[1];
    if (value > 32767) {
        value -= 65536;
    }
    return value;
}

int main(void) {
    // Open I2C device file
    const char *filename = "/dev/i2c-1";
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        perror("Failed to open I2C bus");
        exit(1);
    }

    // Set the I2C slave address
    if (ioctl(fd, I2C_SLAVE, ADS1115_I2C_ADDRESS) < 0) {
        perror("Failed to connect to ADS1115");
        exit(1);
    }

    // Continuously read data from the ADC
    while (1) {
        // Configure ADS1115 for AIN0
        configure_ads1115(fd, 0);
        
        // Read ADC value
        int raw_value = read_ads1115(fd);
        
        // Convert raw value to voltage (based on configured PGA)
        // Current setting PGA=±4.096V, LSB = 8.192V/65535 ≈ 0.000125 V/LSB
        float voltage = (raw_value * 4.096) / 32768.0;
        
        // Print raw value and corresponding voltage
        printf("Raw Value: %d, Voltage: %.3f V\n", raw_value, voltage);
        sleep(1); // Delay for 1 second
    }

    // Close I2C file descriptor
    close(fd);
    return 0;
}

