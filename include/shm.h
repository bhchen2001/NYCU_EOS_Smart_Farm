#pragma once

#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define SHM_KEY "/sensor_data_shm"

/*
 * shared memory for sensor data
 *     - pid: process id of the controller
 */
typedef struct {
    pid_t pid;
    float humidity;
} shared_humidity;

void initialize_shm(int *shm_fd, void **shm_ptr);
void get_humidity(void *shm_ptr, float *humidity);
void set_humidity(void *shm_ptr, float humidity);
void remove_shm(void *shm_ptr);