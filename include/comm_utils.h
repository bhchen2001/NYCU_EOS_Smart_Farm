#pragma once

/*
 * TCP socket related constants
 */
#define BUFFER_SIZE 1024
#define PORT 4444

/*
 * message queue related constants
 */

#define LOW_PRIORITY_MSGQ_KEY 1234
#define HIGH_PRIORITY_MSGQ_KEY 5678

/*
 * client request structure
 *     - msg_type: message type
 *     - client_id: id of the sender
 *     - request_type: type of request (CONTROL, QUERY, ALARM)
 *     - control_signal: control signal for water pump (0=OFF, 1=ON)
 *     - priority: priority of the request (0=LOW, 1=HIGH)
 *     - humidity: current humidity value
 */
typedef struct {
    long msg_type;
    int client_id;
    char request_type[10];
    int control_signal;
    int priority;
    int pump_period;
    float humidity;
} task_request;

/*
 * request_type
 *     - CONTROL: control water pump
 *     - QUERY: query soil moisture
 *     - ALARM: send alarm
 */
#define CONTROL "CONTROL"
#define QUERY "QUERY"
#define ALARM "ALARM"

/*
 * control signal
 *     - PUMP_OFF: turn off the water pump
 *     - PUMP_ON: turn on the water pump
 *     - PUMP_PERIOD: set the period of the water pump
 */
enum {
    PUMP_OFF = 0,
    PUMP_ON,
    PUMP_PERIOD,
    PUMP_OPTION_MAX
};

/*
 * priority level
 *     - LOW_PRIORITY: low priority task
 *         - query soil moisture
 *     - HIGH_PRIORITY: high priority task
 *         - control water pump
 *         - send alarm
 */
enum {
    LOW_PRIORITY = 0,
    HIGH_PRIORITY,
    PRIORITY_MAX
};

void set_task_request(task_request *request, int client_id, const char *request_type, int control_signal, int priority, int pump_period, float humidity);