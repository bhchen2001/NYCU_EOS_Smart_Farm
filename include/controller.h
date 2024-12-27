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

#define SCHED_OTHER_LOW_NICE_VALUE 10
#define SCHED_OTHER_MED_NICE_VALUE 5
#define SCHED_OTHER_HIGH_NICE_VALUE 1

extern int shm_id;
extern shared_humidity *shm_ptr;

extern pthread_mutex_t water_pump_mutex;

extern int l298n_fd;

extern sig_atomic_t shutdown_request;

void busy_wait(int seconds);    // busy wait for priority settng testing
void *high_priority_task();     // high priority task (control water pump from user & send alarm)
void sigusr1_handler();         // signal handler for SIGUSR1 (automatic control of water pump)
void *low_priority_task();      // low priority task (query soil moisture)
void water_pump_mutex_init();   // initialize water pump mutex