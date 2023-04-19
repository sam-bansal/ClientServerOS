#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <unistd.h>

#define SHM_SIZE 1024

struct request
{
    char type; // 'a' for arithmetic operation, 'e' for even/odd check, 'p' for isPrime check
    int operand1;
    int operand2;
    char operator;
};

struct response
{
    int result;
};

int main()
{
    int shmid;
    key_t key;
    struct request *shm_req;
    struct response *shm_res;

    if ((key = ftok("server.c", 'R')) == -1)
    {
        perror("ftok");
        exit(1);
    }

    if ((shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT)) == -1)
    {
        perror("shmget");
        exit(1);
    }

    if ((shm_req = (struct request *)shmat(shmid, NULL, 0)) == (struct request *)-1)
    {
        perror("shmat");
        exit(1);
    }

    shm_res = (struct response *)(shm_req + 1); // The response struct is placed right after the request struct in the shared memory segment

    while (1)
    {
        while (shm_req->type == '\0')
        { // Wait until the client sends a request
            sleep(1);
        }

        if (shm_req->type == 'a')
        { // Arithmetic operation
            int result;
            switch (shm_req->operator)
            {
            case '+':
                result = shm_req->operand1 + shm_req->operand2;
                break;
            case '-':
                result = shm_req->operand1 - shm_req->operand2;
                break;
            case '*':
                result = shm_req->operand1 * shm_req->operand2;
                break;
            case '/':
                result = shm_req->operand1 / shm_req->operand2;
                break;
            default:
                printf("Invalid operator: %c\n", shm_req->operator);
                exit(1);
            }

            shm_res->result = result;
        }
        else if (shm_req->type == 'e')
        { // Even/odd check
            shm_res->result = shm_req->operand1 % 2 == 0 ? 1 : 0;
        }

        else if (shm_req->type == 'p')
        { // isPrime check
            int n = shm_req->operand1;
            int res=1;
            if (n <= 1)
                res = 0;

            for (int i = 2; i * i <= n; i++)
                if (n % i == 0)
                    res = 0;

            
            shm_res->result = res;
            
        }
        else
        {
            printf("Invalid request type: %c\n", shm_req->type);
            exit(1);
        }

        shm_req->type = '\0'; // Reset the request type to indicate that the request has been processed

        printf("Result: %d\n", shm_res->result);
    }

    if (shmdt(shm_req) == -1)
    {
        perror("shmdt");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(1);
    }

    return 0;
}
