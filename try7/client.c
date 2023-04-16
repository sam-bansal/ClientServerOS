#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#define SHARED_MEM_SIZE 1024 // increased shared memory size

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {
    key_t key = ftok("hey", 100);
    int shmid = shmget(key, SHARED_MEM_SIZE, 0666);
    char *shm = (char*) shmat(shmid, (void*) 0, 0);
    char message[256];
    
    while(1) {
        printf("Enter a message: ");
        fgets(message, 256, stdin);

        pthread_mutex_lock(&mutex);
        strncpy(shm, message, strlen(message)); // copy message to shared memory
        pthread_mutex_unlock(&mutex);

        if(strncmp(message, "exit", 4) == 0) break;
    }
    
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    
    return 0;
}
