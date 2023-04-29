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
  
#define PRINT_INFO(MSG, ...) printf("%s INFO %d:%d %ld %s %s %d : " MSG ";;\n", \
                                    "TODO_PRINT_TIME", getpid(), getppid(), pthread_self(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
  
#define PRINT_ERROR(MSG, ...) printf("%s ERROR %d:%d %ld %s %s %d : " MSG ";;\n", \
                                      "TODO_PRINT_TIME", getpid(), getppid(), pthread_self(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
  
char *register_failed = "Registration Unsuccesful";
int count = 0;
int user_count[10];
int i = 1;
pthread_t th[10];
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
struct hash_table *ht;
struct hash_table *ht1;
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
  char username[50];
  int clientId;
};
struct response
{
  int clientId;
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
  
  char *name = (char *)arg;
  int ct = hash_table_get(ht, name);
  // printf("Worker thread ( %p ) created for client id : %d \n", &th[ct], ct);
  PRINT_INFO("Worker Thread %p Created for client id: %d ", &th[ct], ct);
  
  struct request *shm_req = malloc(sizeof(struct request));
  struct response *shm_res = malloc(sizeof(struct response));
  int comm_id;
  key_t comm_key = ftok("client.c", ct);
  
  comm_id = shmget(comm_key, SHM_SIZE, 0666 | IPC_CREAT);
  
  if (comm_id < 0)
  {
    // perror("shmget communication channel");
    PRINT_ERROR("shmget communication channel");
    exit(1);
  }
  
  shm_req = (struct request *)shmat(comm_id, NULL, 0);
  
  if (shm_req == (struct request *)-1)
  {
    perror("shmat communication channel");
    exit(1);
  }
  shm_res = (struct response *)(shm_req + 1);
  
  printf("Communication Channel opened for client %d at Communication id %d\n", ct, comm_id);
  while (1)
  {
    sleep(2);
    while (shm_req->type == 0)
    {
      sleep(1);
    }
    printf("Service Request Received from client \n");
    pthread_mutex_lock(&mutex1);
    printf("THREAD %d HAS LOCKED THE SERVER\n", ct);
    count++;
    printf("Total requests processed by the server is %d \n", count);
    int index = hash(name);
  
    printf("The number of requests sent by  the user %d is %d \n", ct, ++user_count[ct]);
  
    if (shm_req->type == 1)
    {
      printf("Computing the result.... \n");
      if (shm_req->operator== '+')
      {
        int x = shm_req->operand1 + shm_req->operand2;
        shm_req->result = x;
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
      printf("Client getting deleted...... \n");
      char *username = malloc(sizeof(char *));
      for (int i = 0; i < TABLE_SIZE; i++)
      {
        struct node *curr = ht->buckets[i];
        while (curr != NULL)
        {
          if (curr->value == shm_req->operand2)
          {
            strcpy(username, curr->key);
            printf("The user being delete is : %s\n", curr->key);
            break;
          }
          curr = curr->next;
        }
      }
      // printf("The user being delete is : %s\n",username);
      hash_table_delete(ht, username);
      printf("Client  deleted...... \n");
      printf("Current List Of Users\n");
      print_hash_table(ht);
      shm_req->type = 0;
      pthread_mutex_unlock(&mutex1);
      pthread_cancel(pthread_self());
      pthread_exit(NULL);
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
    else if (shm_req->type == 3)
    {
      shm_req->result = is_prime(shm_req->operand1);
    }
    else if (shm_req->type == 4)
    {
      printf("This functionality is not available yet\n");
      shm_req->type = 0;
      pthread_mutex_unlock(&mutex1);
      printf("THREAD %d HAS UNLOCKED THE SERVER\n", ct);
      continue;
    }
    else
    {
      printf("Invalid input entered\n");
      shm_req->type = 0;
      pthread_mutex_unlock(&mutex1);
      printf("THREAD %d HAS UNLOCKED THE SERVER\n", ct);
      continue;
    }
    printf("Server responded to client %d request with result : %d \n", ct, shm_req->result);
    shm_res->result = shm_req->result;
  
    shm_req->clientId = ct;
    shm_req->type = 0;
  
    pthread_mutex_unlock(&mutex1);
    printf("THREAD %d HAS UNLOCKED THE SERVER\n", ct);
  }
}
  
int main()
{
  ht = create_hash_table();
  ht1 = create_hash_table();
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
  PRINT_INFO("Connect Channel opened at address : %p", shm);
  // printf("Connect Channel opened at address : %p\n", shm);
  shm->op = -1;
  strcpy(shm->request, "*");
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(&shm->mutex, &attr);
  pthread_mutexattr_destroy(&attr);
  
  while (1)
  {
    pthread_mutex_lock(&shm->mutex);
    if (strcmp(shm->request, "*") == 0)
    {
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
        shm->response = i;
        hash_table_insert(ht, name, shm->response);
        printf("Current List Of Users\n");
        print_hash_table(ht);
        user_count[i++] = 0;
        if (pthread_create(&th[shm->response - 1], NULL, handle_client, &name) != 0)
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
  }
}