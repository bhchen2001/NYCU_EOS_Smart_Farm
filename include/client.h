#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <signal.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4444
#define BUFFER_SIZE 1024

int sockfd;

void setup_connection();
void sendRequest(const char *requestType, int controlSignal);
void get_alarm();

void setup_connection() {
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

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

void sendRequest(const char *requestType, int controlSignal) {
    char buffer[BUFFER_SIZE] = {0};
    char request[BUFFER_SIZE];

    sprintf(request, "%s %d", requestType, controlSignal);

    // send the request
    printf("Sending request: %s\n", request);
    send(sockfd, request, BUFFER_SIZE, 0);

    // receive the response
    memset(buffer, 0, BUFFER_SIZE);
    read(sockfd, buffer, BUFFER_SIZE);
    printf("Response from server: %s\n", buffer);

    return;
}

void get_alarm() {
    char buffer[BUFFER_SIZE] = {0};

    while (1) {
        // receive the request
        memset(buffer, 0, BUFFER_SIZE);
        read(sockfd, buffer, BUFFER_SIZE);
        printf("Alarm from server: %s\n", buffer);
    }

    return;
}