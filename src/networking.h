#ifndef NETWORKING
#define NETWORKING

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 1024

struct send_file_args{
    char* dest_ip;
    int port;
    char* source_path;
};

void* send_file(void *arg);

#endif