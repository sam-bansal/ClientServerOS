#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024

int main() {
  key_t key = ftok("server.c", 1);
  if (key == -1) {
    perror("ftok");
    exit(EXIT_FAILURE);
  }

  int shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
  if (shm_id == -1) {
    perror("shmget");
    exit(EXIT_FAILURE);
  }

  char *shm_ptr = shmat(shm_id, NULL, 0);
  if (shm_ptr == (char*) -1) {
    perror("shmat");
    exit(EXIT_FAILURE);
  }

  printf("Server is ready. Waiting for a message from the client...\n");

  while (1) {
    while (*shm_ptr == '\0') {
      sleep(1); // Wait for client to write a message
    }
    printf("Message received from the client: %s\n", shm_ptr);
    *shm_ptr = '\0'; // Reset the shared memory
  }

  if (shmdt(shm_ptr) == -1) {
    perror("shmdt");
    exit(EXIT_FAILURE);
  }

  if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
    perror("shmctl");
    exit(EXIT_FAILURE);
  }

  return 0;
}
