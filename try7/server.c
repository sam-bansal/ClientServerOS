#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024

// Define the shared memory structure
typedef struct {
    int client_id;
    char message[SHM_SIZE];
} shared_memory;

// Define the mutex lock
pthread_mutex_t mutex;

void* handle_client(void* arg) {
    // Cast the argument to the shared memory structure
    shared_memory* shm = (shared_memory*) arg;
    
    // Loop to handle client requests
    while (1) {
        // Acquire the lock
        pthread_mutex_lock(&mutex);

        // Check if the client has sent a message
        if (shm->client_id != -1) {
            // Print the message received from the client
            printf("Client %d says: %s\n", shm->client_id, shm->message);

            // Reset the shared memory values
            shm->client_id = -1;
            memset(shm->message, 0, SHM_SIZE);
        }

        // Release the lock
        pthread_mutex_unlock(&mutex);

        // Sleep for some time to avoid busy waiting
        usleep(1000);
    }
}

int main() {
    // Generate the key using ftok
    key_t key = ftok("hey", 100);

    // Allocate the shared memory
    int shmid = shmget(key, sizeof(shared_memory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }

    // Attach to the shared memory
    shared_memory* shm = shmat(shmid, NULL, 0);
    if (shm == (void*) -1) {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }

    // Initialize the shared memory values
    shm->client_id = -1;
    memset(shm->message, 0, SHM_SIZE);

    // Initialize the mutex lock
    pthread_mutex_init(&mutex, NULL);

    // Create the threads to handle client requests
    pthread_t thread;
    if (pthread_create(&thread, NULL, handle_client, (void*) shm) != 0) {
        perror("pthread_create error");
        exit(EXIT_FAILURE);
    }

    // Wait for the threads to finish
    pthread_join(thread, NULL);

    // Detach from the shared memory
    if (shmdt(shm) == -1) {
        perror("shmdt error");
        exit(EXIT_FAILURE);
    }

    // Deallocate the shared memory
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl error");
        exit(EXIT_FAILURE);
    }

    return 0;
}
