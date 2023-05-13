#ifndef NETWORKING
#define NETWORKING

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define SEND_SIZE 1024


struct send_file_args{
    char* dest_ip;
    int port;
    char* source_path;
};

// void* send_file(void *arg);

typedef struct {
    int chunk_num;
    int socketfd;
} send_thread_arg;

void send_file(char* ip, int port, char* filepath, int chunks);


#endif