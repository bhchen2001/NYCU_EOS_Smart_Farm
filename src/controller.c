#include "include/controller.h"

void high_priority_task() {
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
        sprintf(response, "Water pump is %s\n", request.control_signal ? "ON" : "OFF");
        // ********************************************************************

        // send response to client
        if (write(request.client_id, response, BUFFER_SIZE) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
        }
    }
}

void low_priority_task() {
    // set lower priority
    setpriority(PRIO_PROCESS, 0, LOW_PRIORITY);
    client_request request;

    while (1) {
        if (msgrcv(low_priority_msgq, &request, sizeof(client_request) - sizeof(long), 0, 0) < 0) {
            perror("Message receive failed");
            exit(EXIT_FAILURE);
        }
        printf("[Low-Priority] Handling QUERY request: Client=%d\n", request.client_id);

        /*
         ******************************************************************
         * simulate fetching sensor data (integrated with sensor-subsystem)
         */
        char response[BUFFER_SIZE];
        sprintf(response, "Soil Moisture: 45%%\n");
        // ****************************************************************

        // send response to client
        if (write(request.client_id, response, BUFFER_SIZE) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
        }
    }
}