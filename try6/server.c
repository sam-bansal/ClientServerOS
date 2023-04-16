#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#define SHARED_MEM_SIZE 1024
#define MAX_CLIENTS 10

typedef struct
{
    int connected;
    int client_id;
    int request;
    int result;
} client_data;

int main()
{
    // Create shared memory segment for client data
    int shm_fd = shm_open("/server.c", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHARED_MEM_SIZE);
    client_data *clients = (client_data *)mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Create semaphore for shared memory access
    sem_t *sem_id = sem_open("/semaphore", O_CREAT, 0666, 1);

    // Create socket for accepting client connections
    //int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        exit(1);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    // Bind socket to IP address and port
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind");
        exit(1);
    }

    // Listen for incoming client connections
    if (listen(server_fd, 10) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("Server listening on port 8080...\n");

    // Initialize client data
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].connected = 0;
        clients[i].client_id = i;
        clients[i].request = -1;
        clients[i].result = -1;
    }

    // Wait for incoming client connections
    int connected_client_id = -1;
    while (1)
    {
        // Accept client connection
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1)
        {
            perror("accept");
            exit(1);
        }

        printf("Client connected\n");

        // Look for a free client slot in shared memory
        sem_wait(sem_id);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (!clients[i].connected)
            {
                connected_client_id = i;
                break;
            }
        }
        sem_post(sem_id);

        // Fork a child process to handle client request
        pid_t child_pid = fork();
        if (child_pid == -1)
        {
            perror("fork");
            exit(1);
        }
        else if (child_pid == 0)
        {
            // Child process (client handler)
            // Wait for requests from client
            while (1)
            {
                int request;
                if (recv(client_fd, &request, sizeof(int), 0) == -1)
                {
                    perror("recv");
                    exit(1);

                    // Check if client is disconnecting
                    if (request == -1)
                    {
                        printf("Client disconnected\n");
                        clients[connected_client_id].connected = 0;
                        exit(0);
                    }

                    printf("Client %d requested check for %d\n", connected_client_id, request);

                    // Check if number is odd or even
                    int result = (request % 2 == 0) ? 0 : 1;

                    // Update shared memory with client request and result
                    sem_wait(sem_id);
                    clients[connected_client_id].request = request;
                    clients[connected_client_id].result = result;
                    sem_post(sem_id);

                    // Send result to client
                    if (send(client_fd, &result, sizeof(int), 0) == -1)
                    {
                        perror("send");
                        exit(1);
                    }
                    else
                    {
                        // Parent process (server)
                        // Update shared memory with client connection status
                        sem_wait(sem_id);
                        clients[connected_client_id].connected = 1;
                        sem_post(sem_id);
                    }
                }
            }
            // Close socket and free shared memory
            close(server_fd);
            munmap(clients, SHARED_MEM_SIZE);
            shm_unlink("/shared_mem");
            sem_close(sem_id);
            sem_unlink("/semaphore");

            return 0;
        }
    }
}