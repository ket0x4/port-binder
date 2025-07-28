#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "binder.h"

typedef struct _client {
    int client_socket;
    int port; 
} client;

void *handle_client(void *arg) {
    client *c = (client*)arg;
    struct sockaddr_in client_address;
    socklen_t addr_len = sizeof(client_address);

    // Get client information
    getpeername(c->client_socket, (struct sockaddr *)&client_address, &addr_len);
    char *client_ip = inet_ntoa(client_address.sin_addr);
    int client_port = ntohs(client_address.sin_port);

    printf("Client connected: IP = %s, Port = %d\n", client_ip, client_port);

    sock_obj sock = create_socket(c->port);
    if(!sock.status){
        return NULL;
    }

    // Read data from the client
    // Create client port bind
    int addrlen = sizeof(sock.address);
    int new_socket = accept(sock.server_fd, (struct sockaddr*)&sock.address, (socklen_t*)&addrlen);
    // read data from bind
    redirect *p = malloc(sizeof(redirect)*2);
    p[0].fd1 = new_socket;
    p[0].fd2 = c->client_socket;
    p[1].fd1 = c->client_socket;
    p[1].fd2 = new_socket;
    
    pthread_t thread_id1;
    pthread_create(&thread_id1, NULL, async_pipe, (void*)p);
    pthread_t thread_id2;
    pthread_create(&thread_id2, NULL, async_pipe, (void*)(p+1));
    pthread_join(thread_id1, NULL);
    printf("Client disconnected: IP = %s, Port = %d\n", client_ip, client_port);
    close(sock.server_fd);
    return NULL;
}

int main() {

    signal(SIGPIPE, SIG_IGN);

    sock_obj sock = create_socket(SERVER_PORT);

    printf("Server listening on port %d\n", SERVER_PORT);
    int addrlen = sizeof(sock.address);
    while (1) {
        // Accept incoming connections
        client *c = malloc(sizeof(client));
        if ((c->client_socket = accept(sock.server_fd, (struct sockaddr *)&sock.address, (socklen_t*)&addrlen)) >= 0) {
            printf("New connection accepted\n");
            char buffer[BUFFER_SIZE] = {0};
            int readed = read(c->client_socket, buffer, 1024);
            (void) readed;
            if(readed <= 0){
                close(c->client_socket);
                continue;
            }
            c->port = atoi(buffer);
            if (c->port < RANGE_START || c->port > RANGE_END) {
                printf("Rejected port %d: out of allowed range [%d-%d]\n", c->port, RANGE_START, RANGE_END);
                close(c->client_socket);
                free(c);
                continue;
            }
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, handle_client, c);
            pthread_detach(thread_id);
        } else {
            if (errno != EWOULDBLOCK) {
                perror("accept");
            }
        }
        usleep(100000); // Sleep for a short time to avoid busy waiting
    }

    close(sock.server_fd);
    return 0;
}
