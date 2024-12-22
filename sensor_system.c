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
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "include/shm.h"

#define HIGH_PRIORITY_MSGQ_KEY 5678

/*
 * client request structure
 */
typedef struct {
    long msg_type;
    int client_id;
    char request_type[10];
    int control_signal;
    int priority;
} client_request;

int shm_fd;
shared_humidity *shm_ptr;
int high_priority_msgq;

void sigint_handler(int sig) {
    remove_shm((void *)shm_ptr);
    exit(0);
}

/*
 * simulate sensor data update
 */
void timer_handler(int sig) {
    float humidity = (float)(rand() % 100);
    set_humidity((void *)shm_ptr, humidity);
    printf("[SENSOR] Current Humidity: %.2f%%\n", humidity);

    /*
     * if humidity is below 45%
     *     send the request to message queue
     *     signal the controller
     */
    if (humidity < 45.0) {
        client_request request;
        request.msg_type = 1;
        request.client_id = 0;
        strcpy(request.request_type, "ALARM");
        request.control_signal = 0;

        if (msgsnd(high_priority_msgq, &request, sizeof(client_request) - sizeof(long), 0) < 0) {
            perror("Message send failed");
            exit(EXIT_FAILURE);
        }

        printf("[SENSOR] Humidity %.2f%% below threshold -> Sent ALARM request\n", humidity);

        // get the controller pid from shared memory
        pid_t controller_pid = shm_ptr->pid;
        kill(controller_pid, SIGUSR2);
    }
}

int main() {

    // set up signal handler for SIGINT
    signal(SIGINT, sigint_handler);

    // initialize shared memory
    initialize_shm(&shm_fd, (void **)&shm_ptr);
    
    // initialize message queue
    high_priority_msgq = msgget(HIGH_PRIORITY_MSGQ_KEY, 0666 | IPC_CREAT);
    if (high_priority_msgq < 0) {
        perror("Message queue creation failed");
        exit(EXIT_FAILURE);
    }

    // set up timer for sensor data update
    signal(SIGALRM, timer_handler);

    /*
     * set up timer
     *     triggered every 5 second
     */
    struct itimerval timer;
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 5;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    while (1) {
        pause();
    }

    return 0;
}