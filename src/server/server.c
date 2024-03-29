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
    
    if (strlen(command) == 0 ) {
        return SEND_AGAIN;
    }
     
    if (!strcmp(command, "FILECOUNT")) {
        filecount = atoi(content);
        printf("Number of files is: %d\n", filecount);

    } else if (!strcmp(token, "WRITE")) {
        token = strtok(NULL, "|");

        char filename[100];
        strcpy(filename, token);

        strcat(filename, "-");

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

        printf("merging...\n");
        
        if (!merge(token)) {
		    perror("[-]Error in merging.");
            return -1;
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
        sprintf(filename, "%s-%d", filepath, i);
        
		FILE* chunk_fp = fopen(filename, "rb");
		if (chunk_fp == NULL) {
			perror("[-]Error in opening a chunk file");
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

int main(int argc, char* argv[]) {
    char* ip = "127.0.0.1";

    if (argc == 2 && !strcmp(argv[1], "-h")) {
        printf("USER MANUAL FOR SERVER\n########################\n\n-h\t-->\thelp page\n-a[IP]\t-->\tset the IP address, otherwise use loopback IP addr (127.0.0.1)\n");
        exit(0);
    } else if (argc < 2) {
        printf("[!]Set IP to loopback (127.0.0.1)\n");    
    } else {
        ip = argv[1];
        printf("[!]Set IP to (%s)\n", ip);
    }

    int port = 4003;

    int sockfd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[-]Error in server socketfd");
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
    size_t read_size;
    while (filecount) {
        
        if ((read_size=read(client_sock, buffer, SIZE)) > 0) {
            
            while (read_size < 1024) {
                read_size += read(client_sock, buffer + read_size, SIZE - read_size);
            }

            res = command_handler(buffer);

            if (res == DONE) {
                printf("Got a done message.\n");
                filecount--;
            } else if (res == SEND_AGAIN) {                
                if (send(client_sock, "SEND_AGAIN", SIZE, 0) == -1) {
                    perror("[-]Error in sending the number of chunks.");
                    exit(1);
                }
            } else if (res == SUCCESS) {
                if (send(client_sock, "SUCCESS", SIZE, 0) == -1) {
                    perror("[-]Error in sending the number of chunks.");
                    exit(1);
                }
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
