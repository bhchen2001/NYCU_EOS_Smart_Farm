#include "include/device_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

void open_ads1115_device(int *fd_ads) {
    *fd_ads = open(ADS1115_DEV, O_RDONLY);
    if (*fd_ads < 0) {
        perror("Failed to open device");
        exit(EXIT_FAILURE);
    }
    return;
}

void open_l298n_device(int *fd_l298n) {
    *fd_l298n = open(L298N_DEV, O_WRONLY);
    if (*fd_l298n < 0) {
        perror("open l298n");
        exit(EXIT_FAILURE);
    }
    return;
}

void get_humidity_dev(int fd_ads, float *humidity) {
    char data[32];
    ssize_t r = read(fd_ads, data, sizeof(data) - 1);
    if (r > 0) {
        data[r] = '\0';
        int raw_value = atoi(data);

        *humidity = (raw_value * 4.096f) / 32768.0f;
        printf("[SENSOR] Current Humidity: %.2f%%\n", *humidity);
    } else {
        perror("Failed to read data");
        exit(EXIT_FAILURE);
    }
    return;
}

void set_pump(int fd_l298n, char command) {
    if (write(fd_l298n, &command, sizeof(command)) < 0) {
        perror("Failed to write to device");
        exit(EXIT_FAILURE);
    }
    return;
}