#define PORT 8080
#define BUFFER_SIZE 1024

#include <fcntl.h>
#define is_sock(A) ((A > 0) && fcntl(A, F_GETFD) != -1)

#include <stdbool.h>

#include "config.h"

typedef struct _sock {
    struct sockaddr_in address;
    int server_fd;
    char buffer[BUFFER_SIZE];
    bool status;
} sock_obj;

typedef struct _redirect {
    int fd1;
    int fd2;
    bool status;
} redirect;

sock_obj create_socket(int port);
sock_obj connect_socket(char* server, int port);
void *async_pipe(void *arg);

