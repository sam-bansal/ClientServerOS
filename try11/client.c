#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <pthread.h>

#define SHM_SIZE 1024

struct request
{
  pthread_mutex_t mutex2;
  int type;
  int operand1;
  int operand2;
  char operator;
};
struct init
{
  pthread_mutex_t mutex;
  char request[50];
  int response;
  int op;
};

struct response
{
  int result;
};

int main(int c, char *argv[])
{
  int shm_id;
  key_t shm_key;
  shm_key = ftok("server.c", 0);
  shm_id = shmget(shm_key, SHM_SIZE, 0666);
  if (shm_id < 0)
  {
    perror("shmget");
    exit(1);
  }
  struct init *shm;
  // char *shmptr, *temp;
  // shmptr = shmat(shm_id, NULL, 0);
  char username[50];
  strcpy(username, argv[1]);
  // printf("Enter a username: ");
  // scanf("%s", &username);
  shm = (struct init *)shmat(shm_id, NULL, 0);
  if (shm == (struct init *)-1)
  {
    perror("shmat");
    exit(1);
  }
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(&shm->mutex, &attr);
  pthread_mutexattr_destroy(&attr);
  int client_id = 0;
  while (1)
  {
    pthread_mutex_lock(&shm->mutex);
    if (strcmp(shm->request, "*") == 0)
    {
      strcpy(shm->request, username);
      printf("The username is being sent %s\n", shm->request);
      pthread_mutex_unlock(&shm->mutex);
      break;
    }
    else if (shm->request == "!")
    {
      break;
    }
    else
    {
      pthread_mutex_unlock(&shm->mutex);
      continue;
    }
  }
  printf("Hello\n");
  while (1)
  {
    if (strcmp(shm->request, "!") == 0)
    {
      strcpy(shm->request, "*");
      printf("The client id is: %d\n", shm->response);
      client_id = shm->response;
      pthread_mutex_unlock(&shm->mutex);
      break;
    }
    else
    {
      continue;
    }
  }

  int option;

  struct request *shm_req2;
  struct response *shm_res;

  int comm_id2;
  key_t commk2 = ftok("client.c", client_id);
  comm_id2 = shmget(commk2, SHM_SIZE, 0666 | IPC_CREAT);
  printf("Reached y\n");
  if (comm_id2 < 0)
  {
    perror("shmget communication channel");
    exit(1);
  }

  shm_req2 = (struct request *)shmat(comm_id2, NULL, 0);
  printf("Reached x\n");
  if (shm_req2 == (struct request *)-1)
  {
    perror("shmat communication channel");
    exit(1);
  }
  strcpy(shm->request, "*");
  shm_res = (struct response *)(shm_req2 + 1);
  while (1)
  {
    sleep(5);

    printf("Choose an option: \n1.Send a Request\n2.Unregister\n");
    scanf("%d", &option);

    printf("Reached 3 %p\n", shm_res);

    if (option == 1)
    {
      if(shm_res->result==-1){
        break;
      }

      pthread_mutex_lock(&shm->mutex);
      shm->op = client_id;
      printf("Choose One: \n1.Arithmetic\n2.Even/Odd\n3.isPrime\n4.isNegative\n");
      int request_type;
      scanf("%d", &request_type);
      int client_id = 0;

      if (request_type == 1)
      {
        printf("Enter first operand: \n");
        scanf("%d", &shm_req2->operand1);
        printf("Enter second operand: \n");
        scanf("%d", &shm_req2->operand2);
        printf("Enter operator (+, -, *, /): \n");
        scanf(" %c", &shm_req2->operator);
        shm_req2->type = 1;
        getchar();
      }
      else if (request_type == 2)
      {
        printf("Enter a number to check if it's even or odd: ");
        scanf("%d", &shm_req2->operand1);
        shm_req2->type = 2;
        shm_req2->operand2 = 0;
        shm_req2->operator= ' ';
        getchar();
        printf("Reached \n");
      }
      else if (request_type == 3)
      {
        printf("Enter a number to check if it's prime: ");
        scanf("%d", &shm_req2->operand1);
        shm_req2->type = 3;
        shm_req2->operand2 = 0;
        shm_req2->operator= ' ';
        getchar();
      }
      else if (request_type == 4)
      {
        printf("This functionality is not available yet.\n");
        continue;
      }
      else
      {
        printf("Please enter a valid input\n");
      }
      printf("Reached 2\n");
      while (shm_req2->type != 0)
      {
        sleep(1);
      }
      shm->op = -1;
      if (shm_res->result == INT_MAX)
      {
        printf("Divide by Zero Error\n");
      }
      else
      {
        printf("The operands are  %d %d\n", shm_req2->operand1, shm_req2->operand2);

        printf("The result is %d\n", shm_res->result);
      }
      pthread_mutex_unlock(&shm->mutex);
      printf("REACHED \n");
    }
    else if (option == 2)
    {
      // Unregister Code
    }
    else
    {
      // Error in choosing option
      printf("Please Enter a Valid option");
    }
  }
}