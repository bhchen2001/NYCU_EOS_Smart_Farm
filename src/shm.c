#include "include/shm.h"

void initialize_shm(int *shm_fd, void **shm_ptr) {
    *shm_fd = shm_open(SHM_KEY, O_CREAT | O_RDWR, 0666);
    if (*shm_fd < 0) {
        perror("shm_open");
        exit(1);
    }

    ftruncate(*shm_fd, sizeof(shared_humidity));
    *shm_ptr = mmap(NULL, sizeof(shared_humidity), PROT_READ | PROT_WRITE, MAP_SHARED, *shm_fd, 0);
    if (*shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    return;
}

void get_humidity_shm(void *shm_ptr, float *humidity) {
    *humidity = ((shared_humidity *)shm_ptr)->humidity;
    return;
}

void set_humidity_shm(void *shm_ptr, float humidity) {
    pthread_mutex_lock(&((shared_humidity *)shm_ptr)->shm_mutex);
    ((shared_humidity *)shm_ptr)->humidity = humidity;
    pthread_mutex_unlock(&((shared_humidity *)shm_ptr)->shm_mutex);
    return;
}

void set_pump_status(void *shm_ptr, int pump_status) {
    pthread_mutex_lock(&((shared_humidity *)shm_ptr)->shm_mutex);
    ((shared_humidity *)shm_ptr)->pump_status = pump_status;
    pthread_mutex_unlock(&((shared_humidity *)shm_ptr)->shm_mutex);
    return;
}

void get_pump_status(void *shm_ptr, int *pump_status) {
    *pump_status = ((shared_humidity *)shm_ptr)->pump_status;
    return;
}

void remove_shm(void *shm_ptr) {
    munmap(shm_ptr, sizeof(shared_humidity));
    shm_unlink(SHM_KEY);
    return;
}