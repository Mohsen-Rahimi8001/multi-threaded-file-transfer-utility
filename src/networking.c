#include "networking.h"
#include <pthread.h>

#define FILE_NAME_LEN 50

pthread_mutex_t mutex_send_lock;

void *send_chunk(void *arg) {
    send_thread_arg args = *(send_thread_arg *)arg;
    
    free(arg);

    char filename[FILE_NAME_LEN];
    sprintf(filename, "output_file_%d", args.chunk_num);

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        pthread_exit(NULL);
    }
    
    size_t bytes_read;
    char data[SEND_SIZE] = {0};

    while(fgets(data, SEND_SIZE, fp) != NULL) {
        
        char* to_send = malloc(SEND_SIZE + sizeof(char) * 50);
        sprintf(to_send, "%d@", args.chunk_num);
        strcat(to_send, data);

        if (send(args.socketfd, to_send, SEND_SIZE + sizeof(char) * 50, 0) == -1) {
            perror("[-]Error in sending file.");
        }

        free(to_send);
        memset(data, 0, SEND_SIZE); 
    }


    fclose(fp);
    printf("The file of the thread %d is sent succesfully\n", args.chunk_num);
    
    int *result = malloc(sizeof(int));
    *result = args.chunk_num - 1;
    
    pthread_exit(result);
}

void send_file(char* ip, int port, char* filepath, int chunks) {
    int sockfd;
    struct sockaddr_in server_addr;
    FILE *fp;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("[-]Error in socket");
        exit(1);
    }
    printf("[+]Client socket created successfully.\n");

    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = port;
    server_addr.sin_family = AF_INET;

    
    int e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(e == -1) {
        perror("[-]Error in socket");
        exit(1);
    }
    printf("[+]Connected to Server.\n");

    fp = fopen(filepath, "r");
    if (fp == NULL) {
        perror("[-]Error in reading file.");
        exit(1);
    }

    char msg[120];
    char filename[100];

    char* token = strtok(filepath, "/");

    while (token != NULL) {
        strcpy(filename, token);
        token = strtok(NULL, "/");
    }

    strcpy(msg, "FILEPATH@");
    strcat(msg, filename);

    if (send(sockfd, msg, sizeof(msg), 0) == -1) {
        perror("[-]Error in sending the number of chunks.");
        exit(1);
    }


    pthread_t th[chunks];
    for (int i = 0; i < chunks; i++) {
        send_thread_arg* arg = malloc(sizeof(send_thread_arg*));
        arg->chunk_num = i + 1;
        arg->socketfd = sockfd;
        if (pthread_create(th+i, NULL, send_chunk, (void*)arg) != 0) {
            perror("[-]Error creating new thread.");
        }
    }

    int *result;
    for (int i = 0; i < chunks; i++) {
        pthread_join(th[i], (void **)&result);
        if (*result != i) {
            fprintf(stderr, "[-]Error sending chunk %d\n", i);
            exit(1);
        }
        free(result);
    }

    char *done_msg = "DONE@";

    if (send(sockfd, done_msg, strlen(done_msg), 0) != strlen(done_msg)) {
        fprintf(stderr, "Error sending DONE message to server\n");
        exit(1);
    }

    printf("[+]File data sent successfully.\n");

    printf("[+]Closing the connection.\n");
    close(sockfd);
    printf("[+]Connection closed successfully.\n");
}
