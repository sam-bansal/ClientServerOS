// server.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

pthread_rwlock_t rwlock;
int* shared_number;

void* server_thread(void* arg) {
  while (1) {
    // Wait for a write lock
    pthread_rwlock_wrlock(&rwlock);

    // Read the number from shared memory
    int number = *shared_number;

    // Check if the number is even or odd
    char* result;
    if (number % 2 == 0) {
      result = "even";
    } else {
      result = "odd";
    }
    
    // Print the result
    printf("Server: received %d, sending %s\n", number, result);

    // Release the lock
    pthread_rwlock_unlock(&rwlock);
  }

  return NULL;
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

  // Create the server thread
  pthread_t server_tid;
  pthread_create(&server_tid, NULL, server_thread, NULL);

  // Wait for the server thread to finish (never happens in this case)
  pthread_join(server_tid, NULL);

  // Clean up the shared memory object and lock
  pthread_rwlock_destroy(&rwlock);
  munmap(shared_number, sizeof(int));
  close(shm_fd);
  shm_unlink("/myshm");

  return 0;
}
