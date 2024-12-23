#include "include/client.h"

void signal_handler(int sig) {
    // kill the child process
    kill(0, SIGKILL);
    close(sockfd);
    exit(0);
}

int main() {
    int choice;

    // setup connection with the server
    setup_connection();

    // child process to handle alarm requests
    if (fork() == 0) {
        get_alarm();
        exit(0);
    }

    while (1) {
        printf("\nClient Test Program\n");
        printf("1. Control Water Pump (ON)\n");
        printf("2. Control Water Pump (OFF)\n");
        printf("3. Control Water Pump (Period)\n");
        printf("4. Query Soil Moisture\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                // turn on the water pump
                sendRequest(CONTROL, PUMP_ON, 0);
                break;
            case 2:
                // turn off the water pump
                sendRequest(CONTROL, PUMP_OFF, 0);
                break;
            case 3:
                // turn on the water pump with period
                sendRequest(CONTROL, PUMP_PERIOD, 10);
            case 4:
                // query soil moisture
                sendRequest(QUERY, -1, 0);
                break;
            case 5:
                printf("Exiting client.\n");
                kill(0, SIGKILL);
                close(sockfd);
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}
