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

struct port_map {
    int client_port;
	int server_port;
};

void *forward_ports(void *arg) {
    struct port_map *map = (struct port_map *)arg;
    redirect *p = malloc(sizeof(redirect) * 2);
    while(1){
        sock_obj client = connect_socket("127.0.0.1", map->client_port);
        sock_obj server = connect_socket(SERVER_IP, SERVER_PORT);
        if(!(client.status && server.status)){
            printf("[WARN] Failed to connect: local port %d, server port %d\n", map->client_port, map->server_port);
            free(p);
            return NULL;
        }
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%d", map->server_port);
        write(server.server_fd, buffer, 1024);
        puts("Connect");
        printf("[INFO] Connecting: local port %d -> server port %d\n", map->client_port, map->server_port);
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
        printf("[INFO] Disconnected\n");
        close(client.server_fd);
        //close(server.server_fd);
    }
    free(p);
    return NULL;
}

int main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN);
    if(argc == 3 && strcmp(argv[1], "-c") == 0) {
        FILE *conf = fopen(argv[2], "r");
        if(!conf) {
            perror("Failed to open config file");
            return 1;
        }
        char line[128];
        while(fgets(line, sizeof(line), conf)) {
            int client_port, server_port;
            if(sscanf(line, "%d:%d", &client_port, &server_port) == 2) {
                struct port_map *map = malloc(sizeof(struct port_map));
                map->client_port = client_port;
                map->server_port = server_port;
                pthread_t tid;
                pthread_create(&tid, NULL, forward_ports, map);
                pthread_detach(tid);
            }
        }
        fclose(conf);
        while(1) sleep(1); // Keep main alive
        return 0;
    }
    if(argc < 3) {
        printf("[INFO] Usage: port-binder <source> <destination>\n");
    }

    p = malloc(sizeof(redirect));
    while(1){
        sock_obj client = connect_socket("127.0.0.1", atoi(argv[1]));
        sock_obj server = connect_socket(SERVER_IP, SERVER_PORT);
        if(!(client.status && server.status)){
            printf("[WARN] Failed to connect: local port %d, server port %d\n", atoi(argv[1]), atoi(argv[2]));
            return 1;
        }
        char buffer[BUFFER_SIZE];
        strcpy(buffer, argv[2]);
        write(server.server_fd, buffer, 1024);

		puts("Connect");
		printf("[INFO] Connecting: local port %d -> server port %d\n", atoi(argv[1]), atoi(argv[2]));
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
		printf("[INFO] Disconnected\n");
		close(client.server_fd);
		//close(server.server_fd);
	}
    return 0;
}

