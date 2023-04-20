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

int i = 0;

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

// void *handle_client(void *arg)
// {
//   struct request *temp = (struct request *)arg;
//   // printf("The username is: %s\n", temp->username);
//   printf("%d%d\n", temp->operand1, temp->operand2);
//   if (temp->type == 1)
//   {
//     // printf("The username is: %s\n", temp->username);
//     printf("%d %d\n", temp->operand1, temp->operand2);
//     if (temp->operator== '+')
//     {
//       int x = temp->operand1 + temp->operand2;
//       temp->result = x;
//       printf("%d%d\n", temp->operand1, temp->operand2);
//     }
//     else if (temp->operator== '-')
//       temp->result = temp->operand1 - temp->operand2;
//     else if (temp->operator== '*')
//       temp->result = temp->operand1 * temp->operand2;
//     else if (temp->operator== '/')
//     {
//       if (temp->operand2 == 0)
//       {
//         temp->result = INT_MAX;
//       }
//       else
//       {
//         temp->result = temp->operand1 / temp->operand2;
//       }
//     }
//     else
//     {
//       printf("Invalid operator: %c\n", temp->operator);
//     }
//   }
//   else if (temp->type == 2)
//   {
//     if (temp->operand1 % 2 == 0)
//     {
//       temp->result = 1; // Even
//     }
//     else
//     {
//       temp->result = 0; // Odd
//     }
//   }
//   else
//   {
//     temp->result = is_prime(temp->operand1);
//   }
//   // printf("%d \n",resp->result);
//   return (void *)temp;
// }

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
    while (shm->op == -1)
    {
      pthread_mutex_lock(&shm->mutex);
      printf("Inside while %s\n", shm->request);
      if (strcmp(shm->request, "*") == 0)
      {
        printf("check0\n");
        pthread_mutex_unlock(&shm->mutex);
        sleep(10);
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
          // shm->response = hash_table_get(ht, name);
          shm->response = -1;
        }
        else
        {
          printf("The user was created\n");
          shm->response = i++;
          hash_table_insert(ht, name, shm->response);
        }
        strcpy(shm->request, "!");
        pthread_mutex_unlock(&shm->mutex);
        sleep(10);
        continue;
      }
      // if (shm->op != -1)
      //   break;
      printf("Reached 4\n");
      printf("The username of the current client is: %s\n", name);
      strcpy(shm->request, "!");
      printf("The username was captured\n");
    }
    struct request *shm_req;
    struct response *shm_res;
    int comm_id;
    key_t comm_key = ftok("client.c", shm->op);

    comm_id = shmget(comm_key, SHM_SIZE, 0666 | IPC_CREAT);

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
    while (shm_req->type == 0)
    {
      sleep(1);
    }

    printf("Request by user: %s\n",name);

   

    if (shm_req->type == 1)
    {
      switch (shm_req->operator)
      {
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
        if (shm_req->operand2 == 0)
        {
          shm_res->result = INT_MAX;
        }
        else
        {
          shm_res->result = shm_req->operand1 / shm_req->operand2;
        }
        break;
      default:
        printf("Invalid operator: %c\n", shm_req->operator);
        break;
      }
    }

    else if(shm_req->type == 5){
      printf("User Deleted: %s\n",name);
      shm_res->result = -1;
      hash_table_delete(ht,name);
    }
    else if (shm_req->type == 2)
    {
      if (shm_req->operand1 % 2 == 0)
      {
        shm_res->result = 1; // Even
      }
      else
      {
        shm_res->result = 0; // Odd
      }
    }
   
     else
    {
      shm_res->result = is_prime(shm_req->operand1);
    }
    shm_req->type = 0;
    printf("Result: %d\n", shm_res->result);
    sleep(10);
  }
}