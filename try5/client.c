#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_SIZE 1024

struct request {
    char type; // 'a' for arithmetic operation, 'e' for even/odd check
    int operand1;
    int operand2;
    char operator;
};

struct response {
    int result;
};

int main() {
    int shmid;
    key_t key;
    struct request *shm_req;
    struct response *shm_res;

    if ((key = ftok("server.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((shmid = shmget(key, SHM_SIZE, 0666)) == -1) {
        perror("shmget");
        exit(1);
    }

    if ((shm_req = (struct request *)shmat(shmid, NULL, 0)) == (struct request *) -1) {
        perror("shmat");
        exit(1);
    }

    shm_res = (struct response *)(shm_req + 1); // The response struct is placed right after the request struct in the shared memory segment

    while (1) {
        char input[10];
        printf("Enter 'a' for arithmetic operation or 'e' for even/odd check: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // Remove the newline character from the input string

        if (strcmp(input, "a") == 0) {
            printf("Enter first operand: ");
            scanf("%d", &shm_req->operand1);
            printf("Enter second operand: ");
            scanf("%d", &shm_req->operand2);
            printf("Enter operator (+, -, *, /): ");
            scanf(" %c", &shm_req->operator);
            shm_req->type = 'a'; // Set the request type to arithmetic operation
            getchar(); // Consume the newline character left in the input buffer by scanf()
        }
        else if (strcmp(input, "e") == 0) {
            printf("Enter a number to check if it's even or odd: ");
            scanf("%d", &shm_req->operand1);
            shm_req->type = 'e'; // Set the request type to even/odd check
            shm_req->operand2 = 0; // Set the second operand to 0 since it's not needed for this operation
            shm_req->operator = ' '; // Set the operator to a space character since it's not needed for this operation
            getchar(); // Consume the newline character left in the input buffer by scanf()
        }
        else if (strcmp(input, "p") == 0) {
            printf("Enter a number to check if it's prime: ");
            scanf("%d", &shm_req->operand1);
            shm_req->type = 'p'; // Set the request type to even/odd check
            shm_req->operand2 = 0; // Set the second operand to 0 since it's not needed for this operation
            shm_req->operator = ' '; // Set the operator to a space character since it's not needed for this operation
            getchar(); // Consume the newline character left in the input buffer by scanf()
        }
        else {
            printf("Invalid input. Please try again.\n");
            continue;
        }

        while (shm_req->type != '\0') { // Wait until the server processes the request
            sleep(1);
        }

        printf("Result: %d\n", shm_res->result);

        // Set the request type back to null to indicate that the server can accept another request
        shm_req->type = '\0';
    }

    if (shmdt(shm_req) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}
