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
#include <pthread.h>

#include "include/shm.h"
#include "include/comm_utils.h"
#include "include/device_utils.h"

int shm_fd;
shared_humidity *shm_ptr;
int low_priority_msgq, high_priority_msgq;

int ads1115_fd;
int gpio_fd;

// avoiding multiple control requests
int control_signal = 0;

void sigint_handler(int sig) {
    remove_shm((void *)shm_ptr);
    close(ads1115_fd);
    close(gpio_fd);
    msgctl(low_priority_msgq, IPC_RMID, NULL);
    msgctl(high_priority_msgq, IPC_RMID, NULL);
    exit(0);
}

/*
 * routine task triggered by timer
 *     - get humidity from sensor
 *     - send request to controller if humidity is below HUMIDITY_THRESHOLD_LOW
 */
void humidity_check_task() {
    // get humidity from sensor
    float humidity;
    get_humidity_dev(ads1115_fd, &humidity);
    set_humidity_shm((void *)shm_ptr, humidity);
    printf("[SENSOR] Current Humidity: %.2f%%\n", humidity);

    /*
     * if humidity is below 45%
     *     send the request to message queue
     *     signal the controller
     */
    if (humidity < HUMIDITY_THRESHOLD_LOW && control_signal == 0) {
        task_request request;

        // send the control request to controller
        set_task_request(&request, 0, CONTROL, 2, 1, 2, humidity);
        if (msgsnd(high_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0) < 0) {
            perror("Message send failed");
            exit(EXIT_FAILURE);
        }
        printf("[SENSOR] Humidity %.2f%% below threshold -> Sent CONTROL request\n", humidity);

        // send the alarm request to controller
        set_task_request(&request, 0, ALARM, 0, 0, 0, humidity);
        if (msgsnd(low_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0) < 0) {
            perror("Message send failed");
            exit(EXIT_FAILURE);
        }
        printf("[SENSOR] Humidity %.2f%% below threshold -> Sent ALARM request\n", humidity);

        control_signal = 1;
    }
    else if (humidity > HUMIDITY_THRESHOLD_HIGH) {
        task_request request;

        // send the control request to controller
        set_task_request(&request, 0, ALARM, 0, 0, 0, humidity);
        if (msgsnd(low_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0) < 0) {
            perror("Message send failed");
            exit(EXIT_FAILURE);
        }
        printf("[SENSOR] Humidity %.2f%% above threshold -> Sent CONTROL request\n", humidity);
    }
}

/*
 * button check task
 *     - check the button press
 *     - signal the controller
 */
void button_check_task() {
    int val_on, val_off;
    
    read_gpio(gpio_fd, GPIO_PUMP_ON, &val_on);
    read_gpio(gpio_fd, GPIO_PUMP_OFF, &val_off);

    if (val_on == 0 && val_off == 0) { // FORCE RESET
        // signal the controller to operate normally
        set_pump_status((void *)shm_ptr, PUMP_RESET_FORCE);
        pid_t controller_pid = shm_ptr->pid;
        kill(controller_pid, SIGUSR1);
        printf("[SENSOR] Button FORCE RESET pressed\n");
    }
    else if (val_on == 0) { // button FORCE ON
        // signal the controller to abort the current task
        set_pump_status((void *)shm_ptr, PUMP_ON_FORCE);
        pid_t controller_pid = shm_ptr->pid;
        kill(controller_pid, SIGUSR1);
        printf("[SENSOR] Button FORCE ON pressed\n");
    } 
    else if (val_off == 0) { // button FORCE OFF
        // signal the controller to abort the current task
        set_pump_status((void *)shm_ptr, PUMP_OFF_FORCE);
        pid_t controller_pid = shm_ptr->pid;
        kill(controller_pid, SIGUSR1);
        printf("[SENSOR] Button FORCE OFF pressed\n");
    }    
}

void routine_task(int sig) {
    static int counter = 0;

    counter++;

    if (control_signal == 15) {
        control_signal = 0;
    } else if (control_signal > 0) {
        control_signal++;
    }

    button_check_task();
    if (counter % 5 == 0) {
        humidity_check_task();
    }

    return;
}

int main() {

    // set up signal handler for SIGINT
    signal(SIGINT, sigint_handler);

    // initialize shared memory
    initialize_shm(&shm_fd, (void **)&shm_ptr);
    
    // initialize message queue
    high_priority_msgq = msgget(HIGH_PRIORITY_MSGQ_KEY, 0666 | IPC_CREAT);
    if (high_priority_msgq < 0) {
        perror("[SENSOR] Message queue creation failed");
        exit(EXIT_FAILURE);
    }
    low_priority_msgq = msgget(LOW_PRIORITY_MSGQ_KEY, 0666 | IPC_CREAT);
    if (low_priority_msgq < 0) {
        perror("[SENSOR] Message queue creation failed");
        exit(EXIT_FAILURE);
    }

    // open ads1115 device
    open_ads1115_device(&ads1115_fd);

    // open gpio device
    open_gpio_device(&gpio_fd);

    // set up timer for sensor data update
    signal(SIGALRM, routine_task);

    /*
     * set up timer
     *     triggered every 5 second
     */
    struct itimerval timer;
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    while (1) {
        pause();
    }

    return 0;
}