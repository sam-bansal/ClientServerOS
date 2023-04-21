#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#define SHM_SIZE 1024

int main() {
  key_t key = ftok("server.c", 1);
  if (key == -1) {
    perror("ftok");
    exit(EXIT_FAILURE);
  }

  int shm_id = shmget(key, SHM_SIZE, 0666);
  if (shm_id == -1) {
    perror("shmget");
    exit(EXIT_FAILURE);
  }

  char *shm_ptr = shmat(shm_id, NULL, 0);
  if (shm_ptr == (char*) -1) {
    perror("shmat");
    exit(EXIT_FAILURE);
  }

  printf("Client is ready. Please enter a message to send to the server:\n");

  char message[SHM_SIZE];
  fgets(message, SHM_SIZE, stdin);
  message[strcspn(message, "\n")] = '\0'; // Remove newline character

  strcpy(shm_ptr, message);
  printf("Message sent to the server: %s\n", message);

  if (shmdt(shm_ptr) == -1) {
    perror("shmdt");
    exit(EXIT_FAILURE);
  }

  return 0;
}
