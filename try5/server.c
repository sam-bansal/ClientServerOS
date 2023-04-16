#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <math.h>

#define SHM_SIZE 1024

struct request {
    char type; // 'a' for arithmetic operation, 'e' for even/odd check, 'p' for isPrime check
    int operand1;
    int operand2;
    char operator;
};

struct response {
    int result;
};

int is_prime(int n) {
    if (n <= 1) {
        return 0;
    }

    int i;
    for (i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return 0;
        }
    }

    return 1;
}

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
        while (shm_req->type == '\0') { // Wait until a request is made
            sleep(1);
        }

        if (shm_req->type == 'a') { // Arithmetic operation
            switch (shm_req->operator) {
                case '+':
                    shm_res->result = shm_req->operand1 + shm_req->operand2;
                    break;
                case '-':
                    shm_res->result = shm_req->operand1 - shm_req->operand2;
                    break;
                case '*':
                    shm_res->result = shm_req->operand1 * shm_req->operand2;
                    break;
                case '/':
                    shm_res->result = shm_req->operand1 / shm_req->operand2;
                    break;
                default:
                    printf("Invalid operator: %c\n", shm_req->operator);
                    break;
            }
        } else if (shm_req->type == 'e') { // Even/odd check
            if (shm_req->operand1 % 2 == 0) {
                shm_res->result = 1; // Even
            } else {
                shm_res->result = 0; // Odd
            }
        } else if (shm_req->type == 'p') { // isPrime check
            shm_res->result = is_prime(shm_req->operand1);
        }

        shm_req->type = '\0'; // Set the request type back to null to indicate that the request has been processed
    }

    if (shmdt(shm_req) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}