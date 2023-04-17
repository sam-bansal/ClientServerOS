#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
  
#define SHM_SIZE 1024

struct request {
  int type;
  int operand1;
  int operand2;
  char operator;
};
 
struct response {
  int result;
};

pthread_mutex_t mutex;

void* handle_client(void* arg) {
    // Cast the argument to the shared memory structure
    struct request* shm = (struct request*) arg;
    
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

