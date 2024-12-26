#pragma once

#define ADS1115_DEV "/dev/ads1115"
#define L298N_DEV   "/dev/l298n"
#define GPIO_CHIP_DEV   "/dev/gpiochip0"

#define GPIO_PUMP_ON 23
#define GPIO_PUMP_OFF 24

#define DEV_PUMP_ON 'O'
#define DEV_PUMP_OFF 'S'

#define HUMIDITY_THRESHOLD_LOW 30.0
#define HUMIDITY_THRESHOLD_HIGH 60.0

void open_ads1115_device(int *fd_ads);
void open_l298n_device(int *fd_l298n);
void open_gpio_device(int *gpio_fd);

void get_humidity_dev(int fd_ads, float *humidity);
void set_pump(int fd_l298n, char command);
void read_gpio(int gpio_fd, int line, int *value);