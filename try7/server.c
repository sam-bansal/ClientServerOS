#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SHARED_MEM_NAME "/my_shared_mem"
#define MAX_CLIENTS 10

int shared_mem_fd;
int *shared_mem_ptr;
int num_clients = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *print_number(void *arg) {
    int client_id = *(int *)arg;
    printf("Client %d requested to print a number\n", client_id);
    // Generate a random number between 1 and 100
    int num = rand() % 100 + 1;
    printf("Number generated for client %d: %d\n", client_id, num);
    return NULL;
}

int main() {
    // Create shared memory
    shared_mem_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shared_mem_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shared_mem_fd, sizeof(int)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    shared_mem_ptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, 0);
    if (shared_mem_ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    *shared_mem_ptr = 0;
    // Wait for clients to connect
    while (num_clients < MAX_CLIENTS) {
        if (*shared_mem_ptr != num_clients) {
            pthread_mutex_lock(&mutex);
            num_clients = *shared_mem_ptr;
            pthread_mutex_unlock(&mutex);
        }
    }
    // Spawn thread to handle client requests
    pthread_t tid[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&tid[i], NULL, print_number, arg);
    }
    // Wait for threads to finish
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_join(tid[i], NULL);
    }
    // Cleanup
    munmap(shared_mem_ptr, sizeof(int));
    close(shared_mem_fd);
    shm_unlink(SHARED_MEM_NAME);
    return 0;
}