#ifndef NETWORKING
#define NETWORKING

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define SEND_SIZE 1024

typedef struct {
    int chunk_num;
    int socketfd;
} send_thread_arg;

void send_file(char* ip, int port, char* filepath, int chunks);

#endif