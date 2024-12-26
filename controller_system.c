#define _GNU_SOURCE
#include "include/comm.h"
#include "include/controller.h"
#include "include/shm.h"
#include "include/device_utils.h"
#include <sys/wait.h>
#include <signal.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void sigint_handler() {
    // remove message queues
    printf("[CONTROLLER] Removing message queues\n");
    remove_msgq();

    // close the server
    printf("[CONTROLLER] Closing server\n");
    close_fd();

    // remove shared memory
    printf("[CONTROLLER] Removing shared memory\n");
    remove_shm((void *)shm_ptr);

    // destroy the mutex
    printf("[CONTROLLER] Destroying mutex\n");
    pthread_mutex_destroy(&water_pump_mutex);

    // kill all child processes
    printf("[CONTROLLER] Killing child processes\n");
    kill(0, SIGKILL);

    exit(0);
}

int main() {
    // set priority to high
    int current_nice = nice(SCHED_OTHER_HIGH_NICE_VALUE);
    if (current_nice == -1) {
        perror("[CONTROLLER] nice");
        exit(EXIT_FAILURE);
    }

    // set up signal handler for SIGINT
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // initialize message queues
    initialize_msgq();
    printf("[CONTROLLER] Message queues initialized\n");

    // initialize shared memory
    initialize_shm(&shm_id, (void **)&shm_ptr);
    printf("[CONTROLLER] Shared memory initialized\n");

    // initialize shutdown request
    shutdown_request = 0;

    // set the pid in shared memory
    shm_ptr->pid = getpid();

    // open the pump device
    open_l298n_device(&l298n_fd);

    // set up signal handler for SIGUSR1
    signal(SIGUSR1, sigusr1_handler);

    // start the server and handle client requests
    setup_server();

    /*
     * may be needed to set the affinity
     *     - for testing the priority setting
     */
    // cpu_set_t cpu_set;
    // CPU_ZERO(&cpu_set);
    // CPU_SET(0, &cpu_set);
    // sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);

    pthread_t high_priority_thread, low_priority_thread;

    pthread_attr_t high_attr, low_attr;
    pthread_attr_init(&high_attr);
    pthread_attr_init(&low_attr);

    pthread_attr_setinheritsched(&high_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setinheritsched(&low_attr, PTHREAD_EXPLICIT_SCHED);

    pthread_attr_setschedpolicy(&high_attr, SCHED_OTHER);
    pthread_attr_setschedpolicy(&low_attr, SCHED_OTHER);

    // struct sched_param high_priority_param, low_priority_param;
    // high_priority_param.sched_priority = SCHED_OTHER_MED_NICE_VALUE;
    // low_priority_param.sched_priority = SCHED_OTHER_LOW_NICE_VALUE;

    // pthread_attr_setschedparam(&high_attr, &high_priority_param);
    // pthread_attr_setschedparam(&low_attr, &low_priority_param);
    
    pthread_create(&high_priority_thread, &high_attr, high_priority_task, NULL);
    pthread_create(&low_priority_thread, &low_attr, low_priority_task, NULL);

    pthread_attr_destroy(&high_attr);
    pthread_attr_destroy(&low_attr);

    handle_client_requests();

    pthread_join(high_priority_thread, NULL);
    pthread_join(low_priority_thread, NULL);

    return 0;
}
