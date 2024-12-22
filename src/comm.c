#include "include/comm.h"

int high_priority_msgq, low_priority_msgq;

int server_fd = -1;
int client_fd = -1;

void initialize_msgq() {
    high_priority_msgq = msgget(HIGH_PRIORITY_MSGQ_KEY, 0666 | IPC_CREAT);
    low_priority_msgq = msgget(LOW_PRIORITY_MSGQ_KEY, 0666 | IPC_CREAT);

    if (high_priority_msgq < 0 || low_priority_msgq < 0) {
        perror("Message queue creation failed");
        exit(EXIT_FAILURE);
    }
}

void remove_msgq() {
    msgctl(high_priority_msgq, IPC_RMID, NULL);
    msgctl(low_priority_msgq, IPC_RMID, NULL);

    return;
}

void setup_server() {
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    printf("New client connected: %d\n", client_fd);
}

void handle_client_requests() {
    char buffer[BUFFER_SIZE] = {0};
    client_request request;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        memset(&request, 0, sizeof(client_request));
        request.priority = -1;

        if (read(client_fd, buffer, BUFFER_SIZE) <= 0) {
            perror("Client Disconnect");
            return;
        }

        // parse client request
        char request_type[10];
        int signal;
        sscanf(buffer, "%s %d", request_type, &signal);
        printf("New request from client %d: %s %d\n", client_fd, request_type, signal);

        request.client_id = client_fd;
        strcpy(request.request_type, request_type);
        request.control_signal = signal;
        request.priority = (strcmp(request_type, "CONTROL") == 0) ? 1 : 0;

        if (request.priority == 1) {
            request.msg_type = 1;
            if (msgsnd(high_priority_msgq, &request, sizeof(client_request) - sizeof(long), 0) < 0) {
                perror("Message send failed");
                exit(EXIT_FAILURE);
            }
        } else {
            request.msg_type = 1;
            if (msgsnd(low_priority_msgq, &request, sizeof(client_request) - sizeof(long), 0) < 0) {
                perror("Message send failed");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void close_fd() {
    close(server_fd);
    close(client_fd);

    return;
}