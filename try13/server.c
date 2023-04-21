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

int i = 1;
pthread_t th[10];
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
  int *client_id = (int *)arg;
  int ct = *client_id;
  printf("Worker thread ( %p ) created for client id : %d \n", &th[ct], ct);
  // printf("The username is: %s\n", shm_req->username);
  // printf("%d%d\n", shm_req->operand1, shm_req->operand2);
  struct request *shm_req = malloc(sizeof(struct request));
  printf("Thread catch 1\n");
  struct response *shm_res = malloc(sizeof(struct response));
  printf("Thread catch 12\n");
  int comm_id;
  key_t comm_key = ftok("client.c", ct);
  printf("Thread catch 3\n");

  comm_id = shmget(comm_key, SHM_SIZE, 0666 | IPC_CREAT);
  printf("Thread catch 4\n");

  if (comm_id < 0)
  {
    perror("shmget communication channel");
    exit(1);
  }
  printf("Reached 2\n");

  shm_req = (struct request *)shmat(comm_id, NULL, 0);
  printf("Reached x \n");
  if (shm_req == (struct request *)-1)
  {
    perror("shmat communication channel");
    exit(1);
  }
  shm_res = (struct response *)(shm_req + 1);
  printf("Reached 3 \n");
  while (1)
  {

    while (shm_req->type == 0)
    {
      //  printf(" 49 \n");
      sleep(1);
    }
    printf("Reached 41 \n");
    if (shm_req->type == 1)
    {
      // printf("The username is: %s\n", shm_req->username);
      printf("%d %d\n", shm_req->operand1, shm_req->operand2);
      if (shm_req->operator== '+')
      {
        int x = shm_req->operand1 + shm_req->operand2;
        shm_req->result = x;
        printf("%d%d\n", shm_req->operand1, shm_req->operand2);
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
      printf("User Deleted: \n");
      shm_req->type = 0;
      pthread_cancel(pthread_self());
      printf("User Deleted: \n");
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
    printf(" The Result computed is : %d \n", shm_req->result);
    shm_req->type = 0;
  }
}

int main()
{
  struct hash_table *ht = create_hash_table();
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
    printf("Reached 1\n");
    // while (shm->op == -1)

    pthread_mutex_lock(&shm->mutex);
    printf("Inside while %s\n", shm->request);
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
        printf("value of i inside existed: %d\n",i);
        printf("The user already exists\n");
        shm->response = hash_table_get(ht, name);
      }
      else
      { printf("value of i is : %d\n",i);
        printf("The user was created\n");
        shm->response = i++;
        hash_table_insert(ht, name, shm->response);
        printf("Shm response is : %d\n",shm->response);
        if (pthread_create(&th[shm->response - 1], NULL, handle_client, &shm->response) != 0)
        {
          perror("pthread_create error");
          exit(EXIT_FAILURE);
        }
      }
      strcpy(shm->request, "!");
      pthread_mutex_unlock(&shm->mutex);
      sleep(10);
      continue;
    }
    // if (shm->op != -1)
    //   break;
  }
}