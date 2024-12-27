#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>

/*
 * structure to pass arguments to worker thread
 *     - thread_id: unique identifier for the thread
 *     - policy: scheduling policy for the thread
 *     - priority: priority for the thread
 *     - execution_time: the amount of time the thread has been running
 */
typedef struct {
    int thread_id;
    int policy;
    int priority;
    double execution_time;
} thread_args_t;

// global barrier for synchronizing thread starts
pthread_barrier_t barrier;

volatile sig_atomic_t done = 0;

/*
 * Busy wait for a specified duration
 *     - CLOCK_THREAD_CPUTIME_ID: thread-specific CPU-time clock, will stop during context switches
 */
void busy_wait(double seconds) {
    struct timespec start, current;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    while (1) {
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &current);
        double elapsed = (current.tv_sec - start.tv_sec) + (current.tv_nsec - start.tv_nsec) / 1e9;
        if (elapsed >= seconds) break;
    }
}

// worker thread function
void *thread_func(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;

    if (args->policy == SCHED_OTHER) {
        int current_nice = nice(0);
        if (current_nice == -1) {
            perror("nice");
            exit(EXIT_FAILURE);
        }

        int new_nice = nice(args->priority);
        if (new_nice == -1) {
            perror("nice");
            exit(EXIT_FAILURE);
        }
    }
    
    // wait for all threads to be ready
    pthread_barrier_wait(&barrier);

    printf("Thread %d is starting\n", args->thread_id);
    
    // update the execution time of the thread
    struct timespec start, current;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    
    while (!done) {
        // Update elapsed time
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &current);
        double elapsed = (current.tv_sec - start.tv_sec) + (current.tv_nsec - start.tv_nsec) / 1e9;

        args->execution_time = elapsed;
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    int num_threads = 0;
    double testing_time = 0;
    char *policies_str = NULL;
    char *priorities_str = NULL;
    int opt;
    
    // parse command line arguments
    while ((opt = getopt(argc, argv, "n:t:s:p:")) != -1) {
        switch (opt) {
            case 'n':
                num_threads = atoi(optarg);
                break;
            case 't':
                testing_time = atof(optarg);
                break;
            case 's':
                policies_str = optarg;
                break;
            case 'p':
                priorities_str = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -n <num_threads> -t <testing_time> -s <policies> -p <priorities>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // parse policy and priority token
    char *policy_tokens[num_threads];
    char *priority_tokens[num_threads];

    char *policy_token = strtok(policies_str, ",");
    int i = 0;

    while (policy_token != NULL) {
        policy_tokens[i] = policy_token;
        policy_token = strtok(NULL, ",");
        i++;
    }

    char *priority_token = strtok(priorities_str, ",");
    i = 0;

    while (priority_token != NULL) {
        priority_tokens[i] = priority_token;
        priority_token = strtok(NULL, ",");
        i++;
    }
    
    // initialize arrays for threads and their arguments
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_args_t *thread_args = malloc(num_threads * sizeof(thread_args_t));
    
    // initialize barrier
    pthread_barrier_init(&barrier, NULL, num_threads + 1);
    
    // set CPU affinity to CPU 0 for the main thread
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
    
    // create and configure threads
    for (int i = 0; i < num_threads; i++) {
        // set up thread arguments
        thread_args[i].thread_id = i;
        thread_args[i].execution_time = 0;
        
        // parse priority
        thread_args[i].priority = atoi(priority_tokens[i]);

        // set up thread attributes
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        // set scheduling policy and priority
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

        if (strcmp(policy_tokens[i], "NORMAL") == 0) {
            thread_args[i].policy = SCHED_OTHER;
        } else if (strcmp(policy_tokens[i], "FIFO") == 0) {
            thread_args[i].policy = SCHED_RR;
        }

        pthread_attr_setschedpolicy(&attr, thread_args[i].policy);

        // notice that schedparam should be set after the policy
        if (thread_args[i].policy == SCHED_RR) {
            struct sched_param param;
            param.sched_priority = thread_args[i].priority;
            pthread_attr_setschedparam(&attr, &param);
        }

        // create thread
        pthread_create(&threads[i], &attr, thread_func, &thread_args[i]);
        
        // set CPU affinity
        pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpu_set);
        
        // clean up attribute
        pthread_attr_destroy(&attr);
    }

    // wait for all threads to be ready
    pthread_barrier_wait(&barrier);
    
    // terminate threads after 5 seconds
    sleep(testing_time);
    done = 1;

    // wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // show the execution time of each thread
    for (int i = 0; i < num_threads; i++) {
        printf("Thread %d: policy=%s, nice value=%d, execution time=%.2f secs\n", i, policy_tokens[i], thread_args[i].priority, thread_args[i].execution_time);
    }
    
    // clean up
    pthread_barrier_destroy(&barrier);
    free(threads);
    free(thread_args);
    
    return 0;
}