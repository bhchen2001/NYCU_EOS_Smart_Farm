#include "include/controller.h"

int shm_id;
shared_humidity *shm_ptr;
pthread_mutex_t water_pump_mutex;

void busy_wait(int seconds) {
    struct timespec start, current;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    while (1) {
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &current);
        double elapsed = (current.tv_sec - start.tv_sec) + (current.tv_nsec - start.tv_nsec) / 1e9;
        if (elapsed >= seconds) break;
    }
}

void *high_priority_task() {
    printf("[HIGH_PRIORITY] Started\n");
    task_request request;
    memset(&request, 0, sizeof(task_request));

    while (1) {
        if (msgrcv(high_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0, 0) < 0) {
            perror("Message receive failed");
            exit(EXIT_FAILURE);
        }
        printf("[HIGH_PRIORITY] Handling CONTROL request: Client=%d, Signal=%d ,Period=%d\n", request.client_id, request.control_signal, request.pump_period);

        char response[BUFFER_SIZE];

        // send response to client
        if (strcmp(request.request_type, ALARM) == 0) {
            printf("[HIGH_PRIORITY] Sending alarm response to client\n");
            sprintf(response, "ALARM: Humidity is below threshold: %.2f%%\n", request.humidity);
            if (write(client_fd, response, BUFFER_SIZE) < 0) {
                perror("[HIGH_PRIORITY] Write failed");
                exit(EXIT_FAILURE);
            }
        } 
        else if (strcmp(request.request_type, CONTROL) == 0) {
            printf("[HIGH_PRIORITY] Controlling water pump:");
            sprintf(response, "Water pump is now");
            switch (request.control_signal) {
                case PUMP_OFF:
                    printf(" OFF\n");
                    strcat(response, " OFF\n");
                    break;
                case PUMP_ON:
                    printf(" ON\n");
                    strcat(response, " ON\n");
                    break;
                case PUMP_PERIOD:
                    printf(" PERIOD: %d\n", request.pump_period);
                    sprintf(response, " ON with period: %d\n", request.pump_period);
                    break;
            }
            // lock the water pump mutex
            pthread_mutex_lock(&water_pump_mutex);
            // busy wait for simulating water pump control
            busy_wait(5);
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
    task_request request;
    memset(&request, 0, sizeof(task_request));

    while (1) {
        if (msgrcv(low_priority_msgq, &request, sizeof(task_request) - sizeof(long), 0, 0) < 0) {
            perror("[LOW_PRIORITY] Message receive failed");
            exit(EXIT_FAILURE);
        }
        if (strcmp(request.request_type, QUERY) == 0) {
            printf("[LOW_PRIORITY] Handling QUERY request: Client=%d\n", request.client_id);

            // get humidity from shared memory
            float humidity;
            get_humidity((void *)shm_ptr, &humidity);
            printf("[LOW_PRIORITY] Current Humidity: %.2f%%\n", humidity);

            // simulate query overhead
            busy_wait(2);

            // send response to client
            char response[BUFFER_SIZE];
            sprintf(response, "Current Humidity: %.2f%%\n", humidity);
            if (write(request.client_id, response, BUFFER_SIZE) < 0) {
                perror("[LOW_PRIORITY] Write failed");
                exit(EXIT_FAILURE);
            }
        }
    }

    return NULL;
}

void sigusr1_handler() {
    // control the water pump
    printf("[CONTROLLER] Controlling water pump\n");

    // lock the water pump mutex
    pthread_mutex_lock(&water_pump_mutex);
    busy_wait(5);
    // unlock the water pump mutex
    pthread_mutex_unlock(&water_pump_mutex);

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