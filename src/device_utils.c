#include "include/device_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

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

void open_gpio_device(int *gpio_fd) {
    *gpio_fd = open(GPIO_CHIP_DEV, O_RDONLY);
    if (*gpio_fd < 0) {
        perror("open gpiochip0");
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

        double voltage = ((double)raw_value * 4.096f) / 32767.0;
        if (voltage < 0) voltage = 0;
        if (voltage > 5) voltage = 5;

        double h_tmp = (1 - (voltage / 5)) * 100;
        if (h_tmp < 0) h_tmp = 0;
        if (h_tmp > 100) h_tmp = 100;

        *humidity = (float)h_tmp;
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

void read_gpio(int gpio_fd, int line, int *value) {
    struct gpiohandle_request req;
    struct gpiohandle_data data;
    int ret;

    req.lineoffsets[0] = line;
    req.lines = 1;
    req.flags = GPIOHANDLE_REQUEST_INPUT;

    ret = ioctl(gpio_fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
    if (ret < 0) {
        perror("Failed to get line handle");
        exit(EXIT_FAILURE);
    }

    ret = ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
    if (ret < 0) {
        perror("Failed to get line values");
        exit(EXIT_FAILURE);
    }

    close(req.fd);

    *value = data.values[0];
    return;
}