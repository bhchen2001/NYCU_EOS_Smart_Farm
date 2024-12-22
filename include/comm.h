# pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/msg.h>
#include <errno.h>

#define LOW_PRIORITY_MSGQ_KEY 1234
#define HIGH_PRIORITY_MSGQ_KEY 5678

#define BUFFER_SIZE 1024
#define PORT 4444

/*
 * file descriptors for server and client
 */
extern int server_fd;
extern int client_fd;

/*
 * client request structure
 */
typedef struct {
    long msg_type;
    int client_id;
    char request_type[10];
    int control_signal;
    int priority;
} client_request;

/*
 * message queues for high and low priority requests
 */
extern int high_priority_msgq;
extern int low_priority_msgq;

void initialize_msgq();
void remove_msgq();
void setup_server();
void handle_client_requests();
void close_fd();