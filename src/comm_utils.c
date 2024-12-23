#include "include/comm_utils.h"
#include <string.h>

void set_task_request(task_request *request, int client_id, const char *request_type, int control_signal, int priority, int pump_period, float humidity) {
    memset(request, 0, sizeof(task_request));
    request->msg_type = 1;
    request->client_id = client_id;
    strcpy(request->request_type, request_type);
    request->control_signal = control_signal;
    request->priority = priority;
    request->pump_period = pump_period;
    request->humidity = humidity;
    return;
}