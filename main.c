#include "comm.h"
#include "controller.h"
#include <sys/wait.h>

int main() {
    int server_fd, client_fd;

    // initialize pipes for inter-process communication
    initialize_pipes();

    // start the server and handle client requests
    setup_server(&server_fd, &client_fd);

    // create high_priority processes
    pid_t pid1 = fork();
    if (pid1 == 0) {
        // close the write end of the pipe in the child process
        close(high_priority_pipe[1]);
        high_priority_task();
        exit(0);
    }

    // create low_priority processes
    pid_t pid2 = fork();
    if (pid2 == 0) {
        // close the write end of the pipe in the child process
        close(low_priority_pipe[1]);
        low_priority_task();
        exit(0);
    }

    // parent process: close the read end of the pipes
    close(high_priority_pipe[0]);
    close(low_priority_pipe[0]);

    handle_client_requests(client_fd);

    // wait for child processes
    wait(NULL);
    wait(NULL);

    return 0;
}
