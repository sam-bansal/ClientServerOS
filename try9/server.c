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

struct request {
	int type;
	int operand1;
	int operand2;
	char operator;
	char *username;
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

void *handle_client(void *arg) {
	struct request *temp = (struct request *) arg;
	struct response * resp;

	if (temp->type == 1) {
		vprintf("The username is: %s", temp->username);
		switch (temp->operator) {
			case '+':
				resp->result = temp->operand1 + temp->operand2;
				break;
			case '-':
				resp->result = temp->operand1 - temp->operand2;
				break;
			case '*':
				resp->result = temp->operand1 *temp->operand2;
				break;
			case '/':
				if (temp->operand2 == 0) {
					resp->result = INT_MAX;
				}
				else {
					resp->result = temp->operand1 / temp->operand2;
				}

				break;
			default:
				vprintf("Invalid operator: %d\n", temp->operator);
				break;
		}
	}
	else if (temp->type == 2) {
		if (temp->operand1 % 2 == 0) {
			resp->result = 1;	// Even
		}
		else {
			resp->result = 0;	// Odd
		}
	}
	else {
		resp->result = is_prime(temp->operand1);
	}

	temp->type = 0;

	return (void*) resp;
}

int main() {
	printf("On the Server side\n");
	int shm_id;
	key_t shm_key;
	shm_key = ftok("server.c", 5);
	shm_id = shmget(shm_key, SHM_SIZE, IPC_CREAT | 0666);
	if (shm_id < 0) {
		perror("shmget");
		exit(1);
	}

	char *shmptr, *temp;
	shmptr = shmat(shm_id, NULL, 0);
	if (shmptr == (char*) - 1) {
		perror("shmat");
		exit(1);
	}

	*shmptr = '*';
	temp = shmptr;
	while (*temp == '*') {
		sleep(1);
	}

	char username[50];
	strncpy(username, shmptr, 50);
	printf("The username of the current client is: %s", username);
	*shmptr = '!';
	printf("The username was captured");
	int client_id = 0;

	// Communication Channel
	struct request *shm_req;
	struct response *shm_res;
	int comm_id;
	key_t comm_key = ftok("client.c", client_id);
	comm_id = shmget(comm_key, SHM_SIZE, 0666 | IPC_CREAT);
	if (comm_id < 0) {
		perror("shmget communication channel");
		exit(1);
	}

	shm_req = (struct request *) shmat(comm_id, NULL, 0);
	if (shm_req == (struct request *) - 1) {
		perror("shmat communication channel");
		exit(1);
	}

	shm_res = (struct response *)(shm_req + 1);
	while (1) {
		while (shm_req->type == 0) {
			sleep(1);
		}

		pthread_t thread;
		if (pthread_create(&thread, NULL, handle_client, (void*) shm_req) != 0) {
			perror("pthread_create error");
			exit(EXIT_FAILURE);
		}

		void *threadResult;
		pthread_join(thread, threadResult);

		shm_res = (struct response *) threadResult;
		// printf("Result: %d\n", shm_res->result);
	}
}