# pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#include "comm.h"
#include "shm.h"
#include "device_utils.h"

#define SCHED_FIFO_LOW_PRIORITY 10
#define SCHED_FIFO_MED_PRIORITY 20
#define SCHED_FIFO_HIGH_PRIORITY 30

extern int shm_id;
extern shared_humidity *shm_ptr;

extern pthread_mutex_t water_pump_mutex;

extern int l298n_fd;

void busy_wait(int seconds);    // busy wait for priority settng testing
void *high_priority_task();     // high priority task (control water pump from user & send alarm)
void sigusr1_handler();         // signal handler for SIGUSR1 (automatic control of water pump)
void *low_priority_task();      // low priority task (query soil moisture)
void water_pump_mutex_init();   // initialize water pump mutex