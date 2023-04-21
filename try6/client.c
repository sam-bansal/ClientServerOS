#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
 
#define SHARED_MEM_SIZE sizeof(int) * 2  // size of shared memory block
#define SEM_NAME "/semaphore"             // name of semaphore
 
int main() {
    int request, result;
    int shm_fd;
    sem_t *sem_id;
    int *shared_mem;
 
    // Open semaphore
    sem_id = sem_open(SEM_NAME, 0);
    if (sem_id == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
 
    // Open shared memory
    shm_fd = shm_open("/server.c", O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
 
    // Map shared memory
    shared_mem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
 
    // Prompt user for number to check
    printf("Enter a number: ");
    scanf("%d", &request);
 
    // Send number to server
    sem_wait(sem_id);
    shared_mem[0] = request;
    shared_mem[1] = -1;  // signal server that client is waiting for result
    sem_post(sem_id);
 
    // Wait for result from server
    while (shared_mem[1] == -1) {
        usleep(1000);  // sleep for 1 millisecond to avoid busy waiting
    }
 
    // Get result from shared memory
    result = shared_mem[1];
 
    // Print result
    if (result == 0) {
        printf("%d is even\n", request);
    } else {
        printf("%d is odd\n", request);
    }
 
    // Unmap shared memory and close semaphore
    munmap(shared_mem, SHARED_MEM_SIZE);
    shm_unlink("/shared_mem");
    sem_close(sem_id);
 
    return 0;
}