#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
  
#define SHM_SIZE 1024
  
#define PRINT_INFO(MSG, ...) printf("%s INFO %d:%d %ld %s %s %d : " MSG ";;\n", \
                                    "TODO_PRINT_TIME", getpid(), getppid(), pthread_self(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
  
#define PRINT_ERROR(MSG, ...) printf("%s ERROR %d:%d %ld %s %s %d : " MSG ";;\n", \
                                      "TODO_PRINT_TIME", getpid(), getppid(), pthread_self(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
  
struct request
{
  pthread_mutex_t mutex2;
  int type;
  int operand1;
  int operand2;
  char operator;
  int result;
  int count;
  char username[50];
  int clientId;
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
  int clientID;
  int result;
};
void signal_callback_handler(int signum)
{
  
  // printf("\n You are now disconnected, \n you may re-connect with the same username \n");
  PRINT_INFO("\n You are now disconnected, \n you may re-connect with the same username \n");
  exit(signum);
}
  
int main(int c, char *argv[])
{
  signal(SIGINT, signal_callback_handler);
  int shm_id;
  key_t shm_key;
  shm_key = ftok("server.c", 0);
  shm_id = shmget(shm_key, SHM_SIZE, 0666);
  if (shm_id < 0)
  {
    PRINT_ERROR("shmget");
    // perror("shmget");
    exit(1);
  }
  struct init *shm;
  
  char username[50];
  strcpy(username, argv[1]);
  
  shm = (struct init *)shmat(shm_id, NULL, 0);
  if (shm == (struct init *)-1)
  {
    PRINT_ERROR("shmat");
    // perror("shmat");
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
      // printf("The username is being sent %s\n", shm->request);
      PRINT_INFO("Username being sent is %s", shm->request);
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
  // printf("Hello %s\n", shm->request);
  PRINT_INFO("Hello %s", shm->request);
  
  while (1)
  {
    if (strcmp(shm->request, "!") == 0)
    {
      strcpy(shm->request, "*");
      // printf("The client id is: %d\n", shm->response);
      PRINT_INFO("The client id is: %d", shm->response);
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
  if (comm_id2 < 0)
  {
    // perror("shmget communication channel");
    PRINT_ERROR("shmget communication channel");
    exit(1);
  }
  
  shm_req2 = (struct request *)shmat(comm_id2, NULL, 0);
  if (shm_req2 == (struct request *)-1)
  {
    // perror("shmat communication channel");
    PRINT_ERROR("shmat communication channel");
    exit(1);
  }
  strcpy(shm->request, "*");
  shm_req2->type = 0;
  shm_res = (struct response *)(shm_req2 + 1);
  while (1)
  {
    sleep(2);
    printf("Choose an option: \n1.Send a Request\n2.Unregister\n");
    scanf("%d", &option);
  
    if (option == 1)
    {
      shm->op = client_id;
      printf("Choose an operation: \n1.Arithmetic\n2.Even/Odd\n3.isPrime\n4.isNegative\n");
      int request_type;
      scanf("%d", &request_type);
  
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
        // printf("Reached \n");
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
        shm_req2->type = 4;
        continue;
      }
      else
      {
        printf("Please enter a valid input\n");
        continue;
      }
      // printf("Reached 2\n");
      while (shm_req2->type != 0)
      {
        sleep(1);
      }
      if (shm_res->result == INT_MAX)
      {
        printf("Divide by Zero Error\n");
      }
      else
      {
        printf("The operands are  %d %d\n", shm_req2->operand1, shm_req2->operand2);
        printf("YOUR CLIENT ID IS %d \n", client_id);
        if (request_type == 2)
        {
          if (shm_req2->result == 0)
          {
            printf("The number is odd\n");
          }
          else
          {
            printf("The number is even\n");
          }
        }
        else if (request_type == 3)
        {
          if (shm_req2->result == 0)
          {
            printf("The number is not prime\n");
          }
          else
          {
            printf("The number is prime\n");
          }
        }
        else
        {
          PRINT_INFO("Result is %d", shm_req2->result);
        }
        // printf("The result is %d\n", shm_res->result);
      }
    }
    else if (option == 2)
    {
      pthread_mutex_lock(&shm->mutex);
      shm_req2->operand1 = 0;
      shm_req2->type = 5;
      shm_req2->operand2 = client_id;
      shm_req2->operator= ' ';
      strcpy(shm_req2->username, username);
      PRINT_INFO("The user being deleted is : %s", shm_req2->username);
      // printf("The user being delete is : %s\n",shm_req2->username);
      while (shm_req2->type != 0)
      {
        sleep(1);
      }
  
      pthread_mutex_unlock(&shm->mutex);
  
      if (shmdt(shm_req2) < 0)
      {
        // perror("shmdt");
        PRINT_ERROR("shmdt");
      }
  
      if (shmctl(comm_id2, IPC_RMID, NULL) < 0)
      {
        PRINT_ERROR("shmctl");
        // perror("shmctl");
      }
      PRINT_INFO("You have been succesfully uregistered\nThe comm channel for current client has been deleted ");
      // printf("You have been succesfully uregistered\nThe comm channel for current client has been deleted\n");
  
      break;
    }
    else
    {
      // Error in choosing option
      PRINT_INFO("Please Enter a Valid option ");
      // printf("Please Enter a Valid option\n");
    }
  }
}