#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#define SHARED_MEM_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {
    key_t key = ftok("server.c", 100);
    int shmid = shmget(key, SHARED_MEM_SIZE, 0666);
    char *shm = (char*) shmat(shmid, (void*) 0, 0);
    char message[256];
    
    printf("%d", shmid);
    while(1) {
        printf("Enter a message: ");
        fgets(message, 256, stdin);
        pthread_mutex_lock(&mutex);
        strncpy(shm, message, SHARED_MEM_SIZE);
        shm[SHARED_MEM_SIZE - 1] = '\0';
        pthread_mutex_unlock(&mutex);

        if(strncmp(message, "exit", 4) == 0) break;
    }
    
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    
    return 0;
}
