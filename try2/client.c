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

    // Prompt the user for an arithmetic expression
    struct operation op;
    printf("Enter an arithmetic expression (operand1 operator operand2): ");
    scanf("%d %c %d", &op.operand1, &op.operator, &op.operand2);

    // Write the expression to the shared memory segment
    *shm = op;

    // Wait for the server to write the result
    while (shm->operand1 == 0 && shm->operand2 == 0 && shm->operator == '\0') {
        sleep(1);
    }
    sleep(1);
    // Print the result
    printf("Result: %d\n", shm->result);

    // Detach from the shared memory segment
    if (shmdt(shm) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Delete the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

    return 0;
}
