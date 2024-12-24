#pragma once

#define ADS1115_DEV "/dev/ads1115"
#define L298N_DEV   "/dev/l298n"
#define GPIO_CHIP_DEV   "/dev/gpiochip0"

#define DEV_PUMP_ON 'O'
#define DEV_PUMP_OFF 'S'

void open_ads1115_device(int *fd_ads);
void open_l298n_device(int *fd_l298n);

void get_humidity_dev(int fd_ads, float *humidity);

void set_pump(int fd_l298n, char command);