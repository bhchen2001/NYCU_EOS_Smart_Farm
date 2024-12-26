#include "include/controller.h"

int shm_id;
shared_humidity *shm_ptr;
pthread_mutex_t water_pump_mutex;
int l298n_fd;
sig_atomic_t shutdown_request;

void busy_wait(int seconds) {
    struct timespec start, current;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    while (1) {
        // check the shudown request
        if (shutdown_request) {
            printf("[HIGH_PRIORITY] Shutdown request received, stopping the task\n");
            break;
        }
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &current);
        double elapsed = (current.tv_sec - start.tv_sec) + (current.tv_nsec - start.tv_nsec) / 1e9;
        if (elapsed >= seconds) break;
    }
}

void *high_priority_task() {
    printf("[HIGH_PRIORITY] Started\n");

    // set priority to med
    int current_nice = nice(SCHED_OTHER_HIGH_NICE_VALUE);
    if (current_nice == -1) {
        perror("[HIGH_PRIORITY] nice");
        exit(EXIT_FAILURE);
    }
    task_request request;
    memset(&request, 0, sizeof(task_request));

    while (1) {
        if (msgrcv(high_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0, 0) < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("[HIGH_PRIORITY] Message receive failed");
            exit(EXIT_FAILURE);
        }
        printf("[HIGH_PRIORITY] Handling CONTROL request: Client=%d, Signal=%d ,Period=%d\n", request.client_id, request.control_signal, request.pump_period);

        char response[BUFFER_SIZE];

        // send response to client
        if (strcmp(request.request_type, CONTROL) == 0) {
            printf("[HIGH_PRIORITY] Controlling water pump:");
            sprintf(response, "Water pump is now");
            // lock the water pump mutex
            pthread_mutex_lock(&water_pump_mutex);
            switch (request.control_signal) {
                case PUMP_OFF:
                    printf(" OFF\n");
                    strcat(response, " OFF\n");
                    set_pump(l298n_fd, DEV_PUMP_OFF);
                    break;
                case PUMP_ON:
                    printf(" ON\n");
                    strcat(response, " ON\n");
                    set_pump(l298n_fd, DEV_PUMP_ON);
                    break;
                case PUMP_ON_PERIOD:
                    printf(" PERIOD: %d\n", request.pump_period);
                    sprintf(response, " ON with period: %d\n", request.pump_period);
                    set_pump(l298n_fd, DEV_PUMP_ON);
                    busy_wait(request.pump_period);
                    set_pump(l298n_fd, DEV_PUMP_OFF);
                    // clear the msg queue if the request is from user client
                    if (request.client_id == client_fd) {
                        while (msgrcv(high_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0, IPC_NOWAIT) >= 0);
                    }
                    break;
                case PUMP_OFF_PERIOD:
                    printf(" OFF with period: %d\n", request.pump_period);
                    sprintf(response, " OFF with period: %d\n", request.pump_period);
                    set_pump(l298n_fd, DEV_PUMP_OFF);
                    busy_wait(request.pump_period);
                    // clear the msg queue if the request is from user client
                    if (request.client_id == client_fd) {
                        while (msgrcv(high_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0, IPC_NOWAIT) >= 0);
                    }
                    break;
            }
            // unlock the water pump mutex
            pthread_mutex_unlock(&water_pump_mutex);

            printf("[HIGH_PRIORITY] Sending response to client\n");
            if (write(request.client_id, response, BUFFER_SIZE) < 0) {
                perror("[HIGH_PRIORITY] Write failed");
                exit(EXIT_FAILURE);
            }
        }
        else {
            printf("[HIGH_PRIORITY] Invalid request type\n");
        }
    }
    return NULL;
}

void *low_priority_task() {
    printf("[LOW_PRIORITY] Started\n");

    // set priority to low
    int current_nice = nice(SCHED_OTHER_LOW_NICE_VALUE);
    if (current_nice == -1) {
        perror("[LOW_PRIORITY] nice");
        exit(EXIT_FAILURE);
    }

    task_request request;
    memset(&request, 0, sizeof(task_request));

    while (1) {
        if (msgrcv(low_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0, 0) < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("[LOW_PRIORITY] Message receive failed");
            exit(EXIT_FAILURE);
        }

        // send response to client
        char response[BUFFER_SIZE];

        if (strcmp(request.request_type, QUERY) == 0) {
            printf("[LOW_PRIORITY] Handling QUERY request: Client=%d\n", request.client_id);

            // get humidity from shared memory
            float humidity;
            get_humidity_shm((void *)shm_ptr, &humidity);
            printf("[LOW_PRIORITY] Current Humidity: %.2f%%\n", humidity);

            // simulate query overhead
            // busy_wait(2);

            sprintf(response, "Current Humidity: %.2f%%\n", humidity);
            if (write(request.client_id, response, BUFFER_SIZE) < 0) {
                perror("[LOW_PRIORITY] Write failed");
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(request.request_type, ALARM) == 0) {
            printf("[LOW_PRIORITY] Handling ALARM request: Client=%d\n", request.client_id);

            if (request.humidity > HUMIDITY_THRESHOLD_HIGH){
                // send alarm to client (humidity is above threshold
                sprintf(response, "ALARM: Humidity is above threshold: %.2f%%\n", request.humidity);
                if (write(client_fd, response, BUFFER_SIZE) < 0) {
                    perror("[LOW_PRIORITY] Write failed");
                    exit(EXIT_FAILURE);
                }
            }
            else if (request.humidity < HUMIDITY_THRESHOLD_LOW) {
                // send alarm to client (humidity is below threshold
                sprintf(response, "ALARM: Humidity is below threshold: %.2f%%\n", request.humidity);
                if (write(client_fd, response, BUFFER_SIZE) < 0) {
                    perror("[LOW_PRIORITY] Write failed");
                    exit(EXIT_FAILURE);
                }
            }
        } 
    }

    return NULL;
}

void sigusr1_handler() {
    // receive the signal
    printf("[CONTROLLER] SIGUSR1 Received\n");

    // read the pump status
    int pump_status;
    get_pump_status((void *)shm_ptr, &pump_status);

    task_request request;

    switch (pump_status) {
        case PUMP_OFF_FORCE:
            printf("[CONTROLLER] Force OFF Pump\n");
            // set the shutdown request
            shutdown_request = 1;

            // clear the high priority message queue
            while (msgrcv(high_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0, IPC_NOWAIT) >= 0);
            set_pump(l298n_fd, DEV_PUMP_OFF);
            break;
        case PUMP_ON_FORCE:
            printf("[CONTROLLER] Force ON Pump\n");
            // set the shutdown request
            shutdown_request = 1;

            // clear the high priority message queue
            while (msgrcv(high_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0, IPC_NOWAIT) >= 0);
            set_pump(l298n_fd, DEV_PUMP_ON);
            break;
        case PUMP_RESET_FORCE:
            printf("[CONTROLLER] Force RESET Pump\n");
            // clear the high priority message queue
            while (msgrcv(high_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0, IPC_NOWAIT) >= 0);

            set_pump(l298n_fd, DEV_PUMP_OFF);
            // reset the shutdown request
            shutdown_request = 0;
            break;
    }

    return;
}

void water_pump_mutex_init() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);

    // set mutex as process-shared (optional for multi-process scenarios)
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);

    // enable priority inheritance
    pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);

    // initialize the mutex
    pthread_mutex_init(&water_pump_mutex, &attr);

    pthread_mutexattr_destroy(&attr);
    return;
}