// client.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

pthread_rwlock_t rwlock;
int* shared_number;

void* client_thread(void* arg) {
  while (1) {
    // Wait for a write lock
    pthread_rwlock_wrlock(&rwlock);

    // Read a number from the user
    printf("Client: enter a number (0 to exit): ");
    int number;
    scanf("%d", &number);

    // Write the number to shared memory
    *shared_number = number;

    // If the number is 0, exit the program
    if (number == 0) {
      pthread_rwlock_unlock(&rwlock);
      return NULL;
    }

    // Release the lock
    pthread_rwlock_unlock(&rwlock);

    // Wait for a read lock
    pthread_rwlock_rdlock(&rwlock);

    // Read the result from shared memory
    char* result;
    if (*shared_number == 0) {
      // Server exited unexpectedly
      pthread_rwlock_unlock(&rwlock);
      return NULL;
    } else if (*shared_number % 2 == 0) {
      result = "even";
    } else {
      result = "odd";
    }

    // Print the result
    printf("Client: received %s from server\n", result);

    // Release the lock
    pthread_rwlock_unlock(&rwlock);
  }
}

int main() {
  // Open the shared memory object
  int shm_fd = shm_open("/myshm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

  // Size the shared memory object to hold an integer
  ftruncate(shm_fd, sizeof(int));

  // Map the shared memory object into the address space of the process
  shared_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

  // Initialize the shared number to 0
  *shared_number = 0;

  // Initialize the read-write lock
  pthread_rwlock_init(&rwlock, NULL);

  // Start the client thread
  pthread_t client_tid;
  pthread_create(&client_tid, NULL, client_thread, NULL);

  // Wait for the client thread to exit
  pthread_join(client_tid, NULL);

  // Clean up the shared memory object and lock
  pthread_rwlock_destroy(&rwlock);
  munmap(shared_number, sizeof(int));
  close(shm_fd);
  shm_unlink("/myshm");

  return 0;
}
