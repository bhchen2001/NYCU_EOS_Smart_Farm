#ifndef COMM_H
#define COMM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define PORT 4444

/*
 * pipe descriptors
 */
extern int high_priority_pipe[2];
extern int low_priority_pipe[2];
extern int client_fd;

/*
 * client request structure
 */
typedef struct {
    int client_id;
    char request_type[10];
    int control_signal;
    int priority;
} client_request;

void initialize_pipes();
void setup_server(int *server_fd, int *client_fd);
void handle_client_requests(int client_fd);

#endif