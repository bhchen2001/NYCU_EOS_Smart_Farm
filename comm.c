#include "comm.h"

int high_priority_pipe[2];
int low_priority_pipe[2];

void initialize_pipes() {
    if (pipe(high_priority_pipe) < 0 || pipe(low_priority_pipe) < 0) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }
}

void setup_server(int *server_fd, int *client_fd) {
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(*server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    if ((*client_fd = accept(*server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    printf("New client connected: %d\n", *client_fd);
}

void handle_client_requests(int client_fd) {
    char buffer[BUFFER_SIZE] = {0};
    client_request request;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        memset(&request, 0, sizeof(client_request));
        request.priority = -1;

        read(client_fd, buffer, BUFFER_SIZE);

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
            write(high_priority_pipe[1], &request, sizeof(client_request));
        } else {
            write(low_priority_pipe[1], &request, sizeof(client_request));
        }

        // char response[BUFFER_SIZE];
        // snprintf(response, BUFFER_SIZE, "Request received: %s %d\n", request_type, signal);
        // write(client_fd, response, BUFFER_SIZE);

        // close(client_fd);
    }
}