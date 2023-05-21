#ifndef NETWORKING
#define NETWORKING

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define HEADER_SIZE 124
#define CONTENT_SIZE 900

typedef struct {
    int chunk_num;
    int socketfd;
} send_thread_arg;

void send_file(int sockfd, char* filepath, int chunks);

#endif