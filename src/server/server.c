#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SIZE 1024
#define CONTENT_SIZE 900
#define HEADER_SIZE 124

int filecount = -1;
int chunks = -1;

enum COMMAND_TYPE {
    DONE,
    SEND_AGAIN,
    SUCCESS,
};

typedef struct {
    int sockfd;
    int file_number;
} write_file_args;

int command_handler(char*);
int merge(char*);

int is_digit(char* string) {
    if (string == NULL || !strcmp(string, "")) return 0;

    for (unsigned long int i = 0; i < strlen(string); i++)
        if (string[i] < 48 || string[i] > 57)
            return 0;

    return 1;
}

int command_handler(char* string) {

    char command[HEADER_SIZE];
    memcpy(command, string, HEADER_SIZE);

    char* content = string + HEADER_SIZE;

    char* token = strtok(string, "|");

    if (!strcmp(command, "FILECOUNT")) {
        filecount = atoi(content);
        printf("Number of files is: %d\n", filecount);

    } else if (!strcmp(token, "WRITE")) {
        token = strtok(NULL, "|");

        char filename[100];
        strcpy(filename, token);

        strcat(filename, "_");

        token = strtok(NULL, "|");

        strcat(filename, token);

		FILE* fp;
        fp = fopen(filename, "ab");

        if (fp == NULL) {
			perror("[-]Error opening new file.");
            return SEND_AGAIN;
        }

        token = strtok(NULL, "|");
        char* endptr;

        long int LEN = strtol(token, &endptr, 10);

		fwrite(content, 1, LEN - HEADER_SIZE, fp);

        fclose(fp);

    } else if (!strcmp(token, "DONE")) {
        token = strtok(NULL, "|");

        if (!merge(token)) {
		    perror("[-]Error in merging.");
            return SEND_AGAIN;
        }

        return DONE;

    } else if (!strcmp(command, "CHUNKS")) {
		chunks = atoi(content);
		printf("Chunks: %d\n", chunks);
	}

    return SUCCESS;
}

void close_fp(FILE* fp) {
    if (fp != NULL) {
        fclose(fp);
    }
}

int merge(char* filepath) {
	if (chunks < 0) {
		return 0;
	} 

	FILE* fp;
	fp = fopen(filepath, "wb");

	if (fp == NULL) {
		perror("[-]Error in writing in the final file.");
		return 0;
	}	

	for (int i = 1; i < chunks + 1; ++i) {

        char filename[100];
        sprintf(filename, "%s_%d", filepath, i);
        
		FILE* chunk_fp = fopen(filename, "rb");
		if (chunk_fp == NULL) {
            printf("error in chunk %d\n", i);
			perror("[-]Error in opening a chunk file.");
			return 0;
		}

        char buffer[CONTENT_SIZE];
        size_t readByte;
        while ((readByte = fread(buffer, 1, CONTENT_SIZE, chunk_fp)) > 0) {
            fwrite(buffer, 1, readByte, fp);
            memset(buffer, 0, CONTENT_SIZE);
        }

        fclose(chunk_fp);
        char del_command[255];
        strcpy(del_command, "rm ");
        strcat(del_command, filename);

        int r = system(del_command);
        if (r < 0) {
            perror("[-]Error deleting a chunk file");
        }
	}

    fclose(fp);

	return 1;
}

int main() {
    char* ip = "127.0.0.1";
    int port = 10080;

    int sockfd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[-]Error in socket");
        exit(1);
    }
    printf("[+]Server socket created successfully.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[-]Error in bind");
        exit(1);
    }
    printf("[+]Binding successful.\n");

    if (listen(sockfd, 5) == 0) {
        printf("[+]Listening....\n");
    } else {
        perror("[-]Error in listening");
        exit(1);
    }

    client_sock = accept(sockfd, (struct sockaddr*)&client_addr, &addr_size);
    if (client_sock < 0) {
        perror("[-]Failed to accept new connection.\n");
    }

    char buffer[SIZE];
    int res;

    while (filecount) {
        if (read(client_sock, buffer, SIZE) > 0) {

            res = command_handler(buffer);

            if (res == DONE) {
                printf("got the done message.\n");
                filecount--;
            } else if (res == SEND_AGAIN) {
            }

            memset(buffer, 0, SIZE);

        } else {
            perror("[-]Error reading file from client");
			break;
        }

    }

    close(client_sock);

    printf("[+]Data written in the file successfully.\n");

    return 0;
}
