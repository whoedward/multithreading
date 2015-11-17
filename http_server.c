#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include <pthread.h>

#include "thread_pool.h"
#include "seats.h"
#include "util.h"

#define BUFSIZE 1024
#define FILENAMESIZE 100

void shutdown_server(int);
void* handle_request(void*);


int listenfd;

// TODO: Declare your threadpool!
pool_t* threadpool;

int main(int argc,char *argv[])
{
    int flag, num_seats = 20;
    long connfd = 0; //int connfd = 0;
    struct sockaddr_in serv_addr;

    char send_buffer[BUFSIZE];

    listenfd = 0;

    int server_port = 8080;

    if (argc > 1)
    {
        num_seats = atoi(argv[1]);
    }

    if (server_port < 1500)
    {
        fprintf(stderr,"INVALID PORT NUMBER: %d; can't be < 1500\n",server_port);
        exit(-1);
    }

    if (signal(SIGINT, shutdown_server) == SIG_ERR)
        printf("Issue registering SIGINT handler");

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( listenfd < 0 ){
        perror("Socket");
        exit(errno);
    }
    printf("Established Socket: %d\n", listenfd);
    flag = 1;
    setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag) );

    // Load the seats;
    load_seats(num_seats);

    // set server address
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(send_buffer, '0', sizeof(send_buffer));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(server_port);

    // bind to socket
    if ( bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) != 0)
    {
        perror("socket--bind");
        exit(errno);
    }

    listen(listenfd, 10);

    // TODO: Initialize your threadpool!
    threadpool = pool_create(10,50);
    // This while loop "forever", handling incoming connections
    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        //pthread_t thread;
        //pthread_create(&thread, NULL, (void*)handle_request, (void*)connfd);
        pool_add_task(threadpool, (void*)handle_request, (void*)connfd);
    }
}

void* handle_request (void* connfd_origin) {
  int connfd = (int) connfd_origin;
  struct request req;

  parse_request(connfd, &req);
  process_request(connfd, &req);
  close(connfd);

  //return (void*) 0;
  pthread_exit(NULL);
}

void shutdown_server(int signo){
    printf("Shutting down the server...\n");

    pool_destroy(threadpool);
    // TODO: Print stats about your ability to handle requests.
    unload_seats();
    close(listenfd);
    exit(0);
}
