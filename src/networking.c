#include "networking.h"

 
void send_file(char* ip, int port, char* filepath) {
    int e;
    int sockfd;
    struct sockaddr_in server_addr;
    FILE *fp;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("[-]Error in socket");
        exit(1);
    }
    printf("[+]Server socket created successfully.\n");

    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = port;
    server_addr.sin_family = AF_INET;

    e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
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

    char data[SIZE] = {0};
            
    while(fgets(data, SIZE, fp) != NULL) {
        if (send(sockfd, data, sizeof(data), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }
        memset(data, 0, SIZE); 
    }

    printf("[+]File data sent successfully.\n");

    printf("[+]Closing the connection.\n");
    close(sockfd);
    printf("[+]Connection closed successfully.\n");
}
