#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#include "binder.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void set_non_blocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}


sock_obj create_socket(int port){
    sock_obj sock;
    strcpy(sock.buffer,"");
    sock.status = false;
    int opt = 1;
    // Create socket file descriptor
    if ((sock.server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return sock;
    }

    // Set socket options
    if (setsockopt(sock.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the address
    sock.address.sin_family = AF_INET;
    sock.address.sin_addr.s_addr = INADDR_ANY;
    sock.address.sin_port = htons(port);

    if (bind(sock.server_fd, (struct sockaddr *)&sock.address, sizeof(sock.address)) < 0) {
        perror("bind failed");
        return sock;
    }

    // Start listening for incoming connections
    if (listen(sock.server_fd, 3) < 0) {
        perror("listen");
        return sock;
    }
    sock.status = true;
    return sock;
}

sock_obj connect_socket(char* server, int port) {
    sock_obj sock;
    sock.status = false;
    struct sockaddr_in server_addr;

    // Create socket
    sock.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock.server_fd < 0) {
        perror("Socket creation failed");
        return sock;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock.server_fd);
        return sock;
    }

    // Connect to the server
    if (connect(sock.server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock.server_fd);
        return sock;
    }
    sock.status = true;
    return sock;
}

void *async_pipe(void *arg){
    redirect *pp = (redirect*) arg;
    redirect p = pp[0];
    char buffer[BUFFER_SIZE] = {0};
    while(1){
        if(!is_sock(p.fd1)){
            puts("Wait fd1");
        } else if (!is_sock(p.fd2)){
            puts("Wait fd2");
        } else {
            break;
        }
    }
    int bytes_read = read(p.fd1, buffer, BUFFER_SIZE);
    while (bytes_read > 0 && is_sock(p.fd1) && is_sock(p.fd2)) {
        // Send a response back to the client
        write(p.fd2, buffer, bytes_read);
        bytes_read = read(p.fd1, buffer, bytes_read);
    }
    printf("Pipe finish %d %d\n", p.fd1, p.fd2);
    pp[0].status = false;
    return NULL;
}


