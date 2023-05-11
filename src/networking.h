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

void send_file(char* ip, int port, char* filepath);

#endif