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
int client_id;

int main() {
    // Connect to shared memory
    shared_mem_fd = shm_open(SHARED_MEM_NAME, O_RDWR, S_IRUSR | S_IWUSR);
    if (shared_mem_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    shared_mem_ptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, 0);
    if (shared_mem_ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    // Register with server
    pthread_mutex_t *mutex_ptr = (pthread_mutex_t *)(shared_mem_ptr + 1);
    pthread_mutex_lock(mutex_ptr);
    client_id = (*shared_mem_ptr)++;
    pthread_mutex_unlock(mutex_ptr);
    printf("Client %d registered with server\n", client_id);
    // Request server to print a number
    printf("Client %d requesting server to print a number\n", client_id);
    pthread_mutex_lock(mutex_ptr);
    int server_num_clients = *shared_mem_ptr;
    pthread_mutex_unlock(mutex_ptr);
    if (server_num_clients == MAX_CLIENTS) {
        printf("Server is busy, try again later\n");
        exit(EXIT_FAILURE);
    }
    // Wait for server to print number
    while (1) {
        pthread_mutex_lock(mutex_ptr);
        int num_clients = *shared_mem_ptr;
        pthread_mutex_unlock(mutex_ptr);
        if (num_clients == server_num_clients) {
            break;
        }
    }
    printf("Client %d received number from server\n", client_id);
    // Cleanup
    munmap(shared_mem_ptr, sizeof(int) + sizeof(pthread_mutex_t));
    close(shared_mem_fd);
    return 0;
}