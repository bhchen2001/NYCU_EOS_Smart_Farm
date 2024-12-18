#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

#define ADS1115_DEV "/dev/ads1115"
#define L298N_DEV   "/dev/l298n"
#define GPIO_CHIP   "/dev/gpiochip0"

#define GPIO_A 23
#define GPIO_B 24

#define SHM_NAME "/sensor_data_shm"

struct shared_data {
    float humidity;
    char  pump_req;
};

enum {
    PUMP_OFF = 0,
    PUMP_ON  = 1
};

static int pump_state = PUMP_OFF;
static int keep_running = 1;

void sigint_handler(int sig) {
    (void)sig;
    keep_running = 0;
}

int read_gpio_value(int gpio_fd, int line) {
    struct gpiohandle_data data;
    struct gpiohandle_request req;

    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = 1;
    req.lineoffsets[0] = line;

    if (ioctl(gpio_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        perror("ioctl GPIO_GET_LINEHANDLE_IOCTL");
        return -1;
    }

    if (ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) {
        perror("ioctl GPIOHANDLE_GET_LINE_VALUES_IOCTL");
        close(req.fd);
        return -1;
    }

    close(req.fd);
    return data.values[0];
}

int set_pump_state(int fd_l298n, int state) {
    char cmd = (state == PUMP_ON) ? 'O' : 'S';
    if (write(fd_l298n, &cmd, 1) < 0) {
        perror("write to l298n");
        return -1;
    }
    pump_state = state;
    printf("[ACTION] Pump state set to %s\n", state == PUMP_ON ? "ON" : "OFF");
    return 0;
}

int main() {
    signal(SIGINT, sigint_handler);

    int fd_ads = open(ADS1115_DEV, O_RDONLY);
    if (fd_ads < 0) {
        perror("open ads1115");
        return 1;
    }

    int fd_l298n = open(L298N_DEV, O_WRONLY);
    if (fd_l298n < 0) {
        perror("open l298n");
        close(fd_ads);
        return 1;
    }

    int gpio_fd = open(GPIO_CHIP, O_RDONLY);
    if (gpio_fd < 0) {
        perror("open gpiochip0");
        close(fd_ads);
        close(fd_l298n);
        return 1;
    }

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        close(fd_ads);
        close(fd_l298n);
        close(gpio_fd);
        return 1;
    }

    ftruncate(shm_fd, sizeof(struct shared_data));
    struct shared_data *shm_ptr = mmap(NULL, sizeof(struct shared_data),
                                       PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        close(fd_ads);
        close(fd_l298n);
        close(gpio_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    shm_ptr->humidity = 0.0;
    shm_ptr->pump_req = 'N';

    while (keep_running) {
        char buf[16];
        memset(buf, 0, sizeof(buf));
        lseek(fd_ads, 0, SEEK_SET);
        int r = read(fd_ads, buf, sizeof(buf) - 1);

        if (r > 0) {
            int raw_value = atoi(buf);
            double voltage = ((double)raw_value * 4.096) / 32767.0;
            printf("[INFO] Current Voltage: %.2fV	|	", voltage);
            if (voltage < 0) voltage = 0;
            if (voltage > 5) voltage = 5;
            // double humidity = (1 - (voltage / 4.096)) * 100.0;
            double humidity = (1 - (voltage / 5)) * 100.0;
            if (humidity < 0) humidity = 0;
            if (humidity > 100) humidity = 100;
            shm_ptr->humidity = (float)humidity;
            printf("[INFO] Current Humidity: %.2f%%\n", humidity);
        }

        char external_req = shm_ptr->pump_req;
        if (external_req == 'O' || external_req == 'S') {
            set_pump_state(fd_l298n, (external_req == 'O') ? PUMP_ON : PUMP_OFF);
            printf("[REQUEST] External request: Pump set to %s\n", external_req == 'O' ? "ON" : "OFF");
            shm_ptr->pump_req = 'N';
        }

        int valA = read_gpio_value(gpio_fd, GPIO_A);
        int valB = read_gpio_value(gpio_fd, GPIO_B);
        int forced = 0;

        if (valA == 0 && valB == 0) {
            set_pump_state(fd_l298n, PUMP_OFF);
            printf("[FORCE] Both GPIO A & B are LOW -> Force OFF Pump\n");
            forced = 1;
        } else if (valA == 0) {
            set_pump_state(fd_l298n, PUMP_ON);
            printf("[FORCE] GPIO A is LOW -> Force ON Pump\n");
            forced = 1;
        } else if (valB == 0) {
            set_pump_state(fd_l298n, PUMP_OFF);
            printf("[FORCE] GPIO B is LOW -> Force OFF Pump\n");
            forced = 1;
        }

        if (!forced && shm_ptr->humidity < 45.0) {
            set_pump_state(fd_l298n, PUMP_ON);
            printf("[SENSOR] Humidity %.2f%% below threshold -> Turn ON Pump\n", shm_ptr->humidity);
        } else if (!forced && shm_ptr->humidity > 55.0) {
            set_pump_state(fd_l298n, PUMP_OFF);
            printf("[SENSOR] Humidity %.2f%% above threshold -> Turn OFF Pump\n", shm_ptr->humidity);
        }

        sleep(1);
    }

    munmap(shm_ptr, sizeof(struct shared_data));
    shm_unlink(SHM_NAME);
    close(fd_ads);
    close(fd_l298n);
    close(gpio_fd);
    return 0;
}

