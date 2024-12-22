#include "include/controller.h"

int shm_id;
shared_humidity *shm_ptr;

void high_priority_task() {
    printf("[High-Priority] Started\n");
    // set higher priority
    setpriority(PRIO_PROCESS, 0, HIGH_PRIORITY);
    client_request request;

    while (1) {
        if (msgrcv(high_priority_msgq, &request, sizeof(client_request) - sizeof(long), 0, 0) < 0) {
            perror("Message receive failed");
            exit(EXIT_FAILURE);
        }
        printf("[High-Priority] Handling CONTROL request: Client=%d, Signal=%d\n", request.client_id, request.control_signal);

        /*
         **********************************************************************
         * simulate controlling water pump (integrated with actuator-subsystem)
         */
        char response[BUFFER_SIZE];
        // ********************************************************************

        // send response to client
        if (strcmp(request.request_type, "ALARM") == 0) {
            printf("[High-Priority] Sending alarm response to client\n");
            sprintf(response, "ALARM: Humidity below threshold\n");
            if (write(client_fd, response, BUFFER_SIZE) < 0) {
                perror("Write failed");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("[High-Priority] Sending response to client\n");
            sprintf(response, "Water pump is %s\n", request.control_signal ? "ON" : "OFF");
            if (write(request.client_id, response, BUFFER_SIZE) < 0) {
                perror("Write failed");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void low_priority_task() {
    printf("[Low-Priority] Started\n");

    // set lower priority
    setpriority(PRIO_PROCESS, 0, LOW_PRIORITY);
    client_request request;

    while (1) {
        if (msgrcv(low_priority_msgq, &request, sizeof(client_request) - sizeof(long), 0, 0) < 0) {
            perror("[Low-Priority] Message receive failed");
            exit(EXIT_FAILURE);
        }
        printf("[Low-Priority] Handling QUERY request: Client=%d\n", request.client_id);

        /*
         ******************************************************************
         * simulate fetching sensor data (integrated with sensor-subsystem)
         */

        // get humidity from shared memory
        float humidity;
        get_humidity((void *)shm_ptr, &humidity);

        printf("[Low-Priority] Current Humidity: %.2f%%\n", humidity);
        // ****************************************************************

        // send response to client
        char response[BUFFER_SIZE];
        sprintf(response, "Current Humidity: %.2f%%\n", humidity);
        if (write(request.client_id, response, BUFFER_SIZE) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
        }
    }
}

void sigusr2_handler() {
    // control the water pump
    printf("[Controller] Controlling water pump\n");

    return;
}