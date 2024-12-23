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

#include "comm_utils.h"

/*
 * file descriptors for server and client
 */
extern int server_fd;
extern int client_fd;

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