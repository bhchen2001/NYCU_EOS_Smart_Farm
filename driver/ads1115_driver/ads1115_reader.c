#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

volatile int keep_running = 1;

void handle_sigint(int sig) {
    keep_running = 0;
}

int main() {
    int fd = open("/dev/ads1115", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    signal(SIGINT, handle_sigint);

    char data[32];  // 提供足夠的緩衝區

    printf("Reading from ADS1115... Press Ctrl+C to stop.\n");

    while (keep_running) {
        ssize_t r = read(fd, data, sizeof(data) - 1);
        if (r > 0) {
            data[r] = '\0';  // 確保字串結尾
            int raw_value = atoi(data);  // 將字串轉為整數
            
            float voltage = (raw_value * 4.096f) / 32768.0f;
            printf("Raw Value: %d, Voltage: %.3f V\n", raw_value, voltage);
        } else {
            perror("Failed to read data");
            break;
        }
        sleep(1);  // 每秒讀取一次
    }

    close(fd);
    printf("\nProgram terminated.\n");
    return 0;
}
