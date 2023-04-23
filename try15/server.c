    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <unistd.h>
    #include <math.h>
    #include <limits.h>
    #include <pthread.h>
    #include "hashtable.h"
     
    #define SHM_SIZE 1024
     
    char *register_failed = "Registration Unsuccesful";
    struct hash_table *ht;
     
    int i = 1;
    pthread_t th[10];
    pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
    int total_request_count = 0;
     
    struct init
    {
        pthread_mutex_t mutex;
        char request[50];
        int response;
        int op;
    };
     
    struct request
    {
        pthread_mutex_t mutex2;
        int type;
        int operand1;
        int operand2;
        char operator;
        int result;
        int count;
    };
    struct response
    {
        int result;
    };
     
    int is_prime(int n)
    {
        if (n <= 1)
            return 0;
     
        for (int i = 2; i <= sqrt(n); i++)
        {
            if (n % i == 0)
                return 0;
        }
        return 1;
    }
     
    void *handle_client(void *arg)
    {
        // struct request *shm_req = (struct request *)arg;
        char *name = (char*) arg;
        printf("%s\n", name);
        int client_id = hash_table_get(ht, name);
        int ct = client_id;
        printf("Worker thread ( %p ) created for client id : %d \n", &th[ct], ct);
        // printf("The username is: %s\n", shm_req->username);
        // printf("%d%d\n", shm_req->operand1, shm_req->operand2);
        struct request *shm_req = malloc(sizeof(struct request));
        struct response *shm_res = malloc(sizeof(struct response));
        int comm_id;
        key_t comm_key = ftok("client.c", ct);
     
        comm_id = shmget(comm_key, SHM_SIZE, 0666 | IPC_CREAT);
     
        if (comm_id < 0)
        {
            perror("shmget communication channel");
            exit(1);
        }
        // printf("Communication Channel opened for client %d at Communication id %d\n", ct, comm_id);
     
        shm_req = (struct request *)shmat(comm_id, NULL, 0);
        // printf("Reached x \n");
        if (shm_req == (struct request *)-1)
        {
            perror("shmat communication channel");
            exit(1);
        }
        shm_res = (struct response *)(shm_req + 1);
        // printf("Reached 3 \n");
        printf("Communication Channel opened for client %d at Communication id %d\n", ct, comm_id);
        while (1)
        {
     
            while (shm_req->type == 0)
            {
                //  printf(" 49 \n");
                sleep(1);
            }
            printf("Service Request Received from client \n");
            pthread_mutex_lock(&mutex1);
            printf("THREAD %d HAS LOCKED THE SERVER\n", ct);
            
            shm_req->count = shm_req->count + 1;
            if (shm_req->type == 1)
            {
                // printf("The username is: %s\n", shm_req->username);
                // printf("operand1: %d, operand2: %d received\n", shm_req->operand1, shm_req->operand2);
                printf("Computing the result.... \n");
                if (shm_req->operator== '+')
                {
                    int x = shm_req->operand1 + shm_req->operand2;
                    shm_req->result = x;
                    // printf("%d%d\n", shm_req->operand1, shm_req->operand2);
                }
                else if (shm_req->operator== '-')
                    shm_req->result = shm_req->operand1 - shm_req->operand2;
                else if (shm_req->operator== '*')
                    shm_req->result = shm_req->operand1 * shm_req->operand2;
                else if (shm_req->operator== '/')
                {
                    if (shm_req->operand2 == 0)
                    {
                        shm_req->result = INT_MAX;
                    }
                    else
                    {
                        shm_req->result = shm_req->operand1 / shm_req->operand2;
                    }
                }
                else
                {
                    printf("Invalid operator: %c\n", shm_req->operator);
                }
            }
            else if (shm_req->type == 5)
            {
              //  shm_req->type = 0;
                shm_req->count = shm_req->count - 1;
                hash_table_delete(ht, name);
                printf("Client getting deleted...... \n");
                shm_req->type = 0;
                  printf("Client getting deleted...... \n");
                pthread_cancel(pthread_self());
                printf("Client successfully Deleted \n");
                pthread_mutex_unlock(&mutex1);
                free(shm_req);
                free(shm_res);
            }
            else if (shm_req->type == 2)
            {
                if (shm_req->operand1 % 2 == 0)
                {
                    shm_req->result = 1; // Even
                }
                else
                {
                    shm_req->result = 0; // Odd
                }
            }
            else
            {
                shm_req->result = is_prime(shm_req->operand1);
            }
            printf("Server responds with result : %d \n", shm_req->result);
            shm_req->type = 0;
            pthread_mutex_unlock(&mutex1);
            printf("THREAD %d HAS UNLOCKED THE SERVER\n", ct);
            printf("Request Count for client id %d is : %d\n", ct, shm_req->count);
            printf("Total request count is: %d\n",++total_request_count);
        }
    }
     
    int main()
    {
        ht = create_hash_table();
        printf("On the Server side\n");
        int shm_id;
        key_t shm_key;
        shm_key = ftok("server.c", 0);
        shm_id = shmget(shm_key, SHM_SIZE, IPC_CREAT | 0666);
        if (shm_id < 0)
        {
            perror("shmget");
            exit(1);
        }
        char name[50];
        struct init *shm;
        shm = shmat(shm_id, NULL, 0);
        if (shm == (struct init *)-1)
        {
            perror("shmat");
            exit(1);
        }
        printf("Connect Channel opened at address : %p\n", shm);
        shm->op = -1;
        strcpy(shm->request, "*");
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&shm->mutex, &attr);
        pthread_mutexattr_destroy(&attr);
     
        while (1)
        {
            // printf("Reached 1\n");
            // while (shm->op == -1)
     
            pthread_mutex_lock(&shm->mutex);
            // printf("Inside while %s\n", shm->request);
            if (strcmp(shm->request, "*") == 0)
            {
                // printf("check0\n");
                pthread_mutex_unlock(&shm->mutex);
                sleep(2);
                continue;
            }
            else
            {
                printf("check 1\n");
                strncpy(name, shm->request, 50);
                printf("The username received was : %s\n", name);
     
                if (hash_table_get(ht, name) != -1)
                {
                    printf("The user already exists\n");
                    shm->response = hash_table_get(ht, name);
                }
                else
                {
                    printf("The user was created\n");
                    shm->response = i++;
                    hash_table_insert(ht, name, shm->response);
                    if (pthread_create(&th[shm->response - 1], NULL, handle_client, (void*)name) != 0)
                    {
                        perror("pthread_create error");
                        exit(EXIT_FAILURE);
                    }
                }
                strcpy(shm->request, "!");
                pthread_mutex_unlock(&shm->mutex);
                sleep(5);
                continue;
            }
            // if (shm->op != -1)
            //   break;
        }
    }