#ifndef NETWORKING
#define NETWORKING

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define HEADER_SIZE 124
#define CONTENT_SIZE 900

typedef struct {
    int chunk_num;
    int socketfd;
} send_thread_arg;

void send_file(char* ip, int port, char* filepath, int chunks);

#endif