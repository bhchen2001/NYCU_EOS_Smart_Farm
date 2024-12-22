# pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <time.h>
#include <signal.h>

#include "comm.h"
#include "shm.h"

#define LOW_PRIORITY 10
#define HIGH_PRIORITY -10

extern int shm_id;
extern shared_humidity *shm_ptr;

void high_priority_task();
// void sigusr1_handler();
void sigusr2_handler();
void low_priority_task();