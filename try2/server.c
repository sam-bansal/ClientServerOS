#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_SIZE 1024

// Define a struct for the arithmetic operation
struct operation {
    int operand1;
    int operand2;
    char operator;
    int result;
};

int main() {
    int shmid;
    key_t key;
    struct operation *shm;

    // Generate a unique key for the shared memory
    if ((key = ftok("server.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }

    // Create the shared memory segment
    if ((shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

    // Attach to the shared memory segment
    if ((shm = (struct operation *)shmat(shmid, NULL, 0)) == (struct operation *) -1) {
        perror("shmat");
        exit(1);
    }

    // Wait for the client to write an arithmetic expression
    while (shm->operand1 == 0 && shm->operand2 == 0 && shm->operator == '\0') {
        sleep(1);
    }

    // Perform the arithmetic operation
    int result;
    switch (shm->operator) {
        case '+':
            result = shm->operand1 + shm->operand2;
            break;
        case '-':
            result = shm->operand1 - shm->operand2;
            break;
        case '*':
            result = shm->operand1 * shm->operand2;
            break;
        case '/':
            result = shm->operand1 / shm->operand2;
            break;
        default:
            printf("Invalid operator: %c\n", shm->operator);
            exit(1);
    }

    // Write the result to the shared memory segment
    shm->result = result;

    // Detach from the shared memory segment
    if (shmdt(shm) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}
