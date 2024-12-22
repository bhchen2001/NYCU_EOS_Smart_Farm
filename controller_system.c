#include "include/comm.h"
#include "include/controller.h"
#include "include/shm.h"
#include <sys/wait.h>
#include <signal.h>

pid_t pid_high, pid_low;

void sigint_handler() {
    // remove message queues
    printf("[CONTROLLER] Removing message queues\n");
    remove_msgq();

    // close the server
    printf("[CONTROLLER] Closing server\n");
    close_fd();

    // remove shared memory
    printf("[CONTROLLER] Removing shared memory\n");
    remove_shm((void *)shm_ptr);

    // kill all child processes
    printf("[CONTROLLER] Killing child processes\n");
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
    printf("[CONTROLLER] Message queues initialized\n");

    // initialize shared memory
    initialize_shm(&shm_id, (void **)&shm_ptr);
    printf("[CONTROLLER] Shared memory initialized\n");

    // set the pid in shared memory
    shm_ptr->pid = getpid();

    // set up signal handler for SIGUSR2
    signal(SIGUSR2, sigusr2_handler);

    // start the server and handle client requests
    setup_server();

    // create high_priority processes
    pid_high = fork();
    if (pid_high == 0) {
        high_priority_task();
        exit(0);
    }

    // create low_priority processes
    pid_low = fork();
    if (pid_low == 0) {
        low_priority_task();
        exit(0);
    }

    handle_client_requests();

    // wait for child processes
    wait(NULL);
    wait(NULL);

    return 0;
}
