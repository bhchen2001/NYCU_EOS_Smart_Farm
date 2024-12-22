#include "include/comm.h"
#include "include/controller.h"
#include <sys/wait.h>
#include <signal.h>

void sigint_handler() {
    // remove message queues
    printf("[SERVER] Removing message queues\n");
    remove_msgq();

    // close the server
    printf("[SERVER] Closing server\n");
    close_fd();

    // kill all child processes
    printf("[SERVER] Killing child processes\n");
    kill(0, SIGKILL);

    exit(0);
}

int main() {
    // set up signal handler for SIGINT
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // initialize message queues
    initialize_msgq();

    // start the server and handle client requests
    setup_server();

    // create high_priority processes
    pid_t pid1 = fork();
    if (pid1 == 0) {
        high_priority_task();
        exit(0);
    }

    // create low_priority processes
    pid_t pid2 = fork();
    if (pid2 == 0) {
        low_priority_task();
        exit(0);
    }

    handle_client_requests();

    // wait for child processes
    wait(NULL);
    wait(NULL);

    return 0;
}
