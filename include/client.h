#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <signal.h>

#include "comm_utils.h"

#define SERVER_IP "127.0.0.1"

int sockfd;

void setup_connection();
void sendRequest(const char *requestType, int controlSignal, int pumpPeriod);
void get_alarm();

void setup_connection() {
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    return;
}

void sendRequest(const char *requestType, int controlSignal, int pumpPeriod) {
    // char buffer[BUFFER_SIZE] = {0};
    char request[BUFFER_SIZE];

    sprintf(request, "%s %d %d", requestType, controlSignal, pumpPeriod);

    // send the request
    printf("Sending request: %s\n", request);
    send(sockfd, request, BUFFER_SIZE, 0);

    return;
}

void get_alarm() {
    char buffer[BUFFER_SIZE] = {0};

    while (1) {
        // receive the request
        memset(buffer, 0, BUFFER_SIZE);
        read(sockfd, buffer, BUFFER_SIZE);
        
        // if the answer is alarm
        if (strstr(buffer, ALARM) != NULL) {
            printf("Received alarm: %s\n", buffer);
        } else {
            printf("Received response: %s\n", buffer);
        }
    }

    return;
}