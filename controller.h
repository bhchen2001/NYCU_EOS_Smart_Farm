#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <time.h>

#include "comm.h"

#define LOW_PRIORITY 10
#define HIGH_PRIORITY -10

void busy_wait(int seconds); // just for simulation
void high_priority_task();
void low_priority_task();

#endif