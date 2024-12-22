#include "include/client.h"

int main() {
    int choice;

    // setup connection with the server
    setup_connection();

    while (1) {
        printf("\nClient Test Program\n");
        printf("1. Control Water Pump (ON)\n");
        printf("2. Control Water Pump (OFF)\n");
        printf("3. Query Soil Moisture\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                // turn on the water pump
                sendRequest("CONTROL", 1);
                break;
            case 2:
                // turn off the water pump
                sendRequest("CONTROL", 0);
                break;
            case 3:
                // query soil moisture
                sendRequest("QUERY", 0);
                break;
            case 4:
                printf("Exiting client.\n");
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}
