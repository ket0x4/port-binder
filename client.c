#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#include "binder.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

redirect *p;

int main() {
    signal(SIGPIPE, SIG_IGN);

    p = malloc(sizeof(redirect));
    while(1){
        sock_obj client = connect_socket("127.0.0.1", 22);
        sock_obj server = connect_socket(SERVER_IP, SERVER_PORT);		
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
		close(client.server_fd);
		//close(server.server_fd);
	}
    return 0;
}

