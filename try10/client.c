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
 
#define TABLE_SIZE 100
 
struct node
{
  char *key;
  int value;
  struct node *next;
};
 
struct hash_table
{
  struct node **buckets;
};
 
// hash function
int hash(char *key)
{
  int hash_value = 0;
  for (int i = 0; i < strlen(key); i++)
  {
    hash_value += key[i];
  }
  return hash_value % TABLE_SIZE;
}
 
// create a new node
struct node *create_node(char *key, int value)
{
  struct node *new_node = malloc(sizeof(struct node));
  new_node->key = strdup(key);
  new_node->value = value;
  new_node->next = NULL;
  return new_node;
}
 
// create a new hash table
struct hash_table *create_hash_table()
{
  struct hash_table *new_table = malloc(sizeof(struct hash_table));
  new_table->buckets = calloc(TABLE_SIZE, sizeof(struct node *));
  return new_table;
}
 
// insert a key-value pair into the hash table
void hash_table_insert(struct hash_table *ht, char *key, int value)
{
  int index = hash(key);
  struct node *head = ht->buckets[index];
  struct node *curr = head;
  while (curr != NULL)
  {
    if (strcmp(curr->key, key) == 0)
    {
      curr->value = value;
      return;
    }
    curr = curr->next;
  }
  struct node *new_node = create_node(key, value);
  new_node->next = head;
  ht->buckets[index] = new_node;
}
 
// get the value associated with a key in the hash table
int hash_table_get(struct hash_table *ht, char *key)
{
  int index = hash(key);
  struct node *curr = ht->buckets[index];
  while (curr != NULL)
  {
    if (strcmp(curr->key, key) == 0)
    {
      return curr->value;
    }
    curr = curr->next;
  }
  return -1; // key not found
}
 
// delete a key-value pair from the hash table
void hash_table_delete(struct hash_table *ht, char *key)
{
  int index = hash(key);
  struct node *head = ht->buckets[index];
  if (head == NULL)
  {
    return; // key not found
  }
  if (strcmp(head->key, key) == 0)
  {
    ht->buckets[index] = head->next;
    free(head->key);
    free(head);
    return;
  }
  struct node *prev = head;
  struct node *curr = head->next;
  while (curr != NULL)
  {
    if (strcmp(curr->key, key) == 0)
    {
      prev->next = curr->next;
      free(curr->key);
      free(curr);
      return;
    }
    prev = curr;
    curr = curr->next;
  }
}
 
char *register_failed = "Registration Unsuccesful";
 
int i = 0;
 
struct init
{
  pthread_mutex_t mutex;
  char request[50];
  int response;
  int op;
 
 
};
 
struct request {
	int type;
	int operand1;
	int operand2;
	char operator;
 
 
};
struct response {
  int result;
};
 
 
int is_prime(int n) {
	if (n <= 1)
		return 0;
 
	for (int i = 2; i <= sqrt(n); i++) {
		if (n % i == 0)
			return 0;
	}
	return 1;
}
 
 
 
int main() {
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
  // char *shmptr, *temp;
  // shmptr = shmat(shm_id, NULL, 0);
  shm = shmat(shm_id, NULL, 0);
  shm->op=-1;
  strcpy(shm->request, "*");
  // shm->response = -1;
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
 
	// Communication Channel
	
    
 
	while(1)
	{
    printf("Reached 1\n");
	while(shm->op==-1)
	{
	//printf("Inside while\n");
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
        shm->response = hash_table_get(ht, name);
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
	if(shm->op !=-1)
	break;
	printf("Reached 4\n");
    printf("The username of the current client is\n");
	printf("The username of the current client is: %s\n", name);
	strcpy(shm->request,"!");
	//printf("%d \n",shm_req->type);
	printf("The username was captured\n");
	}
      
	   
	struct request *shm_req;
	struct response *shm_res;
	int comm_id;
	key_t comm_key = ftok("client.c", shm->op);
   
	comm_id = shmget(comm_key, SHM_SIZE, 0666 | IPC_CREAT);
 
	 printf("Reached y %d\n",comm_id);
	if (comm_id < 0) {
		perror("shmget communication channel");
		exit(1);
	}
    printf("Reached 2\n");
 
	shm_req = (struct request *) shmat(comm_id, NULL, 0);
	 printf("Reached x %p\n",shm_req);
	if (shm_req == (struct request *) - 1) {
		perror("shmat communication channel");
		exit(1);
	}
	shm_res = (struct response *)(shm_req + 1);
    printf("Reached 3 %p\n",shm_res);
    printf(" SHARED MEMORY USED IS %p\n",shm_req);
	  while(shm_req->type==0)
    {
       sleep(1);
    }
	
 
		
			
        printf("Reached 4\n");
 
		pthread_t thread;
       // printf("The username is: %s\n", shm_req->username);
	   printf("%d %d\n", shm_req->operand1,shm_req->operand2);
	   shm_res->result= shm_req->operand1+shm_req->operand2;
	
	    shm_req->type=0;
		 printf("Result: %d\n", shm_res->result);
		    shm->op =-1;
	
    
	}
    
}