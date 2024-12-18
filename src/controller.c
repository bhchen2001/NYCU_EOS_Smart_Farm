#include "controller.h"

void busy_wait(int seconds) {
    struct timespec start, current;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    while (1) {
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &current);
        double elapsed = (current.tv_sec - start.tv_sec);
        if (elapsed >= seconds) break;
    }
}

void high_priority_task() {
    // set higher priority
    // setpriority(PRIO_PROCESS, 0, HIGH_PRIORITY);
    client_request request;

    while (1) {
        read(high_priority_pipe[0], &request, sizeof(client_request));
        printf("[High-Priority] Handling CONTROL request: Client=%d, Signal=%d\n", request.client_id, request.control_signal);

        /*
         **********************************************************************
         * simulate controlling water pump (integrated with actuator-subsystem)
         */
        char response[BUFFER_SIZE];
        sprintf(response, "Water pump is %s\n", request.control_signal ? "ON" : "OFF");
        // ********************************************************************

        // send response to client
        write(request.client_id, response, BUFFER_SIZE);
        // close(request.client_id);
    }
}

void low_priority_task() {
    // set lower priority
    // setpriority(PRIO_PROCESS, 0, LOW_PRIORITY);
    client_request request;

    while (1) {
        read(low_priority_pipe[0], &request, sizeof(client_request));
        printf("[Low-Priority] Handling QUERY request: Client=%d\n", request.client_id);

        /*
         ******************************************************************
         * simulate fetching sensor data (integrated with sensor-subsystem)
         */
        char response[BUFFER_SIZE];
        sprintf(response, "Soil Moisture: 45%%\n");
        // ****************************************************************

        // send response to client
        write(request.client_id, response, BUFFER_SIZE);
        // close(request.client_id);
    }
}