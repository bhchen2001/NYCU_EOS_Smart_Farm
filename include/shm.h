#pragma once

#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define SHM_KEY "/sensor_data_shm"

/*
 * shared memory for sensor data
 *     - pid: process id of the controller
 *     - humidity: current humidity value
 */
typedef struct {
    pid_t pid;
    float humidity;
    int pump_status;
    pthread_mutex_t shm_mutex;
} shared_humidity;

/*
 * pump_status
 */
enum {
    PUMP_OFF_FORCE = 0,
    PUMP_ON_FORCE = 1,
    PUMP_RESET_FORCE = 2,
};

void initialize_shm(int *shm_fd, void **shm_ptr);
void get_humidity_shm(void *shm_ptr, float *humidity);
void set_humidity_shm(void *shm_ptr, float humidity);
void set_pump_status(void *shm_ptr, int pump_status);
void get_pump_status(void *shm_ptr, int *pump_status);
void remove_shm(void *shm_ptr);