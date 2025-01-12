#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#include "binder.h"

redirect *p;

int main(int argc, char** argv) {
    if(argc < 3) {
        puts("Usage: port-binder <source> <destination>");
    }
    signal(SIGPIPE, SIG_IGN);

    p = malloc(sizeof(redirect));
    while(1){
        sock_obj client = connect_socket("127.0.0.1", atoi(argv[1]));
        sock_obj server = connect_socket(SERVER_IP, SERVER_PORT);
        if(!(client.status && server.status)){
            return 1;
        }
        char buffer[BUFFER_SIZE];
        strcpy(buffer, argv[2]);
        write(server.server_fd, buffer, BUFFER_SIZE);

		puts("Connect");
		p[0].fd1 = server.server_fd;
		p[0].fd2 = client.server_fd;
		p[1].fd1 = client.server_fd;
		p[1].fd2 = server.server_fd;
	    p[0].status = true;
	    p[1].status = true;
		pthread_t thread_id1;
		pthread_create(&thread_id1, NULL, async_pipe, (void*)p);
		pthread_t thread_id2;
		pthread_create(&thread_id2, NULL, async_pipe, (void*)(p+1));
		while(p[0].status && p[1].status){
		    usleep(1000000);
		}
		puts("disconnect");
		close(client.server_fd);
		//close(server.server_fd);
	}
    return 0;
}

