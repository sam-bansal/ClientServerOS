#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
  
#define SHM_SIZE 1024
  
struct request {
  int type;
  int operand1;
  int operand2;
  char operator;
};
  
struct response {
  int result;
};
  
int main(char argc, char *argv[]) {
  int shm_id;
  key_t shm_key;
  shm_key = ftok("server.c", 5);
  shm_id = shmget(shm_key, SHM_SIZE, 0666);
  if (shm_id < 0) {
    perror("shmget");
    exit(1);
  }

  char *shmptr;
  shmptr = shmat(shm_id, NULL, 0);
  if (shmptr == (char *)-1) {
    perror("shmat");
    exit(1);
  }
  
  char *username = argv[1];
  char *temp = shmptr;
  strcpy(shmptr, username);
  // while (*shmptr != '!')
  // { 
  //   printf("catch\n");
  //   sleep(1);
  // }
  int client_id = 0;
  struct request *shm_req;
  struct response *shm_res;
  int comm_id;
  key_t comm_key = ftok("client.c", client_id);
  comm_id = shmget(comm_key, SHM_SIZE, 0666 | IPC_CREAT);
  if (comm_id < 0)
  {
    perror("shmget communication channel");
    exit(1);
  }
  shm_req = (struct request *)shmat(comm_id, NULL, 0);
  if (shm_req == (struct request *)-1)
  {
    perror("shmat communication channel");
    exit(1);
  }
  shm_res = (struct response *)(shm_req + 1);
  while (1)
  {
    int option;
    printf("Choose an option: \n1.Send a Request\n2.Unregister\n");
    scanf("%d", &option);
    if (option == 1)
    {
      printf("Choose One: \n1.Arithmetic\n2.Even/Odd\n3.isPrime\n4.isNegative\n");
      int request_type;
      scanf("%d", &request_type);
      if (request_type == 1)
      {
        printf("Enter first operand: ");
        scanf("%d", &shm_req->operand1);
        printf("Enter second operand: ");
        scanf("%d", &shm_req->operand2);
        printf("Enter operator (+, -, *, /): ");
        scanf(" %c", &shm_req->operator);
        shm_req->type = 1;
        getchar();
      }
      else if (request_type == 2)
      {
        printf("Enter a number to check if it's even or odd: ");
        scanf("%d", &shm_req->operand1);
        shm_req->type = 2;
        shm_req->operand2 = 0;
        shm_req->operator= ' ';
        getchar();
      }
      else if (request_type == 3)
      {
        printf("Enter a number to check if it's prime: ");
        scanf("%d", &shm_req->operand1);
        shm_req->type = 3;
        shm_req->operand2 = 0;
        shm_req->operator= ' ';
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
      while (shm_req->type != 0)
      {
        sleep(1);
      }
      if (shm_res->result == INT_MAX)
      {
        printf("Divide by Zero Error\n");
      }
      else
      {
        printf("The result is %d\n", shm_res->result);
      }
      shm_req->type = 0;
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