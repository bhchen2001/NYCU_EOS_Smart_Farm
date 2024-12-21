#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define DEVICE_PATH "/dev/l298n"

int main(int argc, char *argv[]) {
    int fd;
    char command;

    if (argc != 2) {
        printf("Usage: %s <O|S>\n", argv[0]);
        return EXIT_FAILURE;
    }

    command = argv[1][0];
    if (command != 'O' && command != 'S') {
        printf("Invalid command! Use 'O' for ON and 'S' for OFF.\n");
        return EXIT_FAILURE;
    }

    fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    if (write(fd, &command, sizeof(command)) < 0) {
        perror("Failed to write to device");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Command '%c' sent successfully.\n", command);
    close(fd);
    return EXIT_SUCCESS;
}

