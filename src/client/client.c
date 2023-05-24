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
#include <float.h>


#define BUFFER_SIZE 1024
#define HEADER_SIZE 124
#define CONTENT_SIZE 900
#define FILE_NAME_LEN 50
#define PORT 10087


typedef struct {
    int chunk_num;
    int socketfd;
    char* filename;
} send_thread_arg;

char* GetFileName(char* filepath) {
    char* filename = malloc(sizeof(char) * 100);
    char* token = strtok(filepath, "/");

    while (token != NULL) {
        strcpy(filename, token);
        token = strtok(NULL, "/");
    }

    return filename;
}

unsigned long int FindSize(char* path)
{
    FILE* file = fopen(path, "rb"); 

    fseek(file, 0L, SEEK_END); 
    long int size = ftell(file); 
    fclose(file); 

    return size;
}


void FileSplitter(char* filePath, int partitions, char* filename)
{
    FILE* input_file = fopen(filePath, "rb"); 
    if (input_file == NULL) {
        printf("Error opening input file.\n");
        exit(1);
    }
    unsigned long int file_size = FindSize(filePath);
    unsigned long int CHUNK_SIZE = (file_size / partitions);
    int BUF_SIZE = CHUNK_SIZE > 2048 ? 1024 : CHUNK_SIZE;

    for (int i = 1; i < partitions + 1; ++i) {   
        char buffer[BUF_SIZE];

        FILE* output_file;
        char output_file_name[50];
        sprintf(output_file_name, "%s_%d", filename, i);
        output_file = fopen(output_file_name, "wb");
        
        if (output_file == NULL) {
            printf("Error creating output file %d.\n", i);
            exit(1);
        }
        
        long int threashold = CHUNK_SIZE * i + 1;
        size_t readBytes;
        
        while (ftell(input_file) < threashold &&
         (readBytes = fread(buffer, 1, BUF_SIZE, input_file)) > 0) {
            fwrite(buffer, 1, readBytes, output_file);            
        }

        fclose(output_file);
    }
    fclose(input_file); 
}


int log2ME(unsigned int x) {
    if (x == 0)
        return -1; // Handle the case of x = 0 or negative numbers (error condition)

    int result = -1;
    while (x != 0) {
        x >>= 1;
        result++;
    }
    return result;
}


int logarithm(int num) {
    double x = (double) num;
    double result = log2ME(x);

    if (result > (int) result){
        return(result+1);
    }

    return ((int) result);
}


bool isNumeric(const char* word) {
    int length = strlen(word);
    if (length == 0) {
        return false;
    }

    int i = 0;
    if (word[0] == '-' || word[0] == '+') {
        i++;
    }

    bool hasDigits = false;
    bool hasDecimal = false;

    for (; i < length; i++) {
        if (isdigit(word[i])) {
            hasDigits = true;
        } else if (word[i] == '.' && !hasDecimal) {
            hasDecimal = true;
        } else {
            return false;
        }
    }

    return hasDigits;
}


void *send_chunk(void *arg) {
    send_thread_arg args = *(send_thread_arg *)arg;
    
    free(arg);

    char filename[FILE_NAME_LEN];
    sprintf(filename, "%s_%d", args.filename, args.chunk_num);

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        pthread_exit(NULL);
    }
    
    size_t bytes_read;
    char data[CONTENT_SIZE] = {0};
    
    char number[10];
    sprintf(number, "%d", args.chunk_num);

    while((bytes_read = fread(data, 1, CONTENT_SIZE, fp)) > 0) {

        size_t toSendLength = bytes_read + HEADER_SIZE;        
        char num[HEADER_SIZE];

        sprintf(num, "WRITE|%s|%d|%ld", args.filename, args.chunk_num, toSendLength);
        
        char* toSend = (char*)malloc(toSendLength + 1);

        memcpy(toSend, num, HEADER_SIZE);
        
        memcpy(toSend + HEADER_SIZE, data, bytes_read);
        
        toSend[toSendLength] = '\0';

        if (send(args.socketfd, toSend, BUFFER_SIZE, 0) == -1) {
            perror("[-]Error in sending file.");
        }

        free(toSend);
        memset(data, 0, CONTENT_SIZE);
    }

    fclose(fp);
    printf("The file of the thread %d is sent succesfully\n", args.chunk_num);
    
    int *result = malloc(sizeof(int));
    *result = args.chunk_num - 1;
    
    pthread_exit(result);
}


void send_file(int sockfd, char* filepath, int chunks) 
{
    printf("> send file > %s\n", filepath);
    FILE *fp;
    fp = fopen(filepath, "rb");
    if (fp == NULL) {
        perror("[-]Error in reading file.");
        exit(1);
    }

    char msg[BUFFER_SIZE];
    char* filename = GetFileName(filepath);

    char num[CONTENT_SIZE];
    sprintf(num, "%d", chunks);

    memcpy(msg, "CHUNKS", HEADER_SIZE);
    memcpy(msg + HEADER_SIZE, num, CONTENT_SIZE);

    msg[BUFFER_SIZE] = '\0';

    if (send(sockfd, msg, BUFFER_SIZE, 0) == -1) {
        perror("[-]Error in sending the number of chunks.");
        exit(1);
    }

    pthread_t th[chunks];
    for (int i = 0; i < chunks; i++) {
        send_thread_arg* arg = malloc(sizeof(send_thread_arg*));
        arg->chunk_num = i + 1;
        arg->socketfd = sockfd;
        arg->filename = filename;
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

    memset(msg, 0, BUFFER_SIZE);

    sprintf(msg, "DONE|%s", filename);
    printf("sending the done message\n");
    if (send(sockfd, msg, BUFFER_SIZE, 0) == -1) {
        fprintf(stderr, "Error sending DONE message to server\n");
        exit(1);
    }
}

int send_file_count(int sockfd, int filecount) {
    char buffer[1024];
    char fileCountStr[CONTENT_SIZE];
    sprintf(fileCountStr, "%d", filecount);

    memcpy(buffer, "FILECOUNT", HEADER_SIZE);
    memcpy(buffer + HEADER_SIZE, fileCountStr, CONTENT_SIZE);

    if (send(sockfd, buffer, BUFFER_SIZE, 0) == -1) {
        perror("[-]Error in sending number of files");
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[]) 
{
    printf("> argc: %d\n", argc);
    // command:
    // ./main 127.0.0.1 -r pat patt pattt -c 10

    if (argc < 3) {
        printf("[-]Not enough arguments.\n");
        return 1;
    }

    char* dest_ip = argv[1];
    printf("> ip: %s\n", dest_ip);

    char* source_path;
    int chunks = -1;

    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("[-]Error in client socketfd");
        exit(1);
    }
    printf("[+]Client socket created successfully.\n");

    server_addr.sin_addr.s_addr = inet_addr(dest_ip);
    server_addr.sin_port = PORT;
    server_addr.sin_family = AF_INET;
    
    int e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(e == -1) {
        perror("[-]Error in socket connection");
        exit(1);
    }
    printf("[+]Connected to Server.\n");

    bool hasChunks;
    if (strcmp(argv[argc - 2],"-c") == 0)
        if (isNumeric(argv[argc - 1])) {
            hasChunks = true;
            chunks = atoi(argv[argc - 1]);
        }
        else {
            printf("[-]Invalid chunck number\n");
            exit(1);  
            }
    else {
        hasChunks = false;
    }

    printf("> has -c: %d\n", hasChunks);
    printf("> chunks: %d\n", chunks);
    
    int number_of_files = 0;
    if (!hasChunks) number_of_files += 9;
    if (strcmp(argv[2],"-r") != 0) number_of_files++;
    else number_of_files += hasChunks ? argc - 5 : argc - 3;

    send_file_count(sockfd, number_of_files);

    if (!hasChunks){
        printf("> Partition 1\n");
        // now should test that how many chuncks can have the best speed...
        // need speed test file (name: SpeedTester)

        source_path = "xSPEEDTEST";
        FILE* ftestp = fopen(source_path, "w");
        for (int i = 0; i < 100000; ++i) {
            fputc('#', ftestp);
        }
        fclose(ftestp);

        int speedtestChunks[] = {1,2,4,8,16,24,32,45,64};
        double min = DBL_MAX;
        int min_ind = -1;
        for (int speed = 0 ; speed < 9 ; speed++)
        {
            clock_t start = clock();

            FileSplitter(source_path, speedtestChunks[speed], source_path);

            send_file(sockfd, source_path, speedtestChunks[speed]);

            clock_t end = clock();
            double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;

            if (elapsed_time < min) {
                min = elapsed_time;
                min_ind = speed;
            }
        }

        chunks = speedtestChunks[min_ind];
        
        char command[50];
        sprintf(command, "rm %s*", source_path);
        if (system(command) < 0) {
            perror("[-]Failed to remove SPEEDTEST chunk files.");
        }
    }


    if (strcmp(argv[2],"-r") != 0)
    {
        printf("> Partition 2\n");
        source_path = argv[2];        
        char* filename = GetFileName(source_path);

        FileSplitter(source_path, chunks, filename);
        send_file(sockfd, source_path, chunks);

        printf("[+]File data sent successfully.\n");
        printf("[+]Closing the connection.\n");
        close(sockfd);
        printf("[+]Connection closed successfully.\n");

        char command[255];
        sprintf(command, "rm %s_*", filename);
        
        if (system(command) < 0) {
            perror("[-]Error in deleting chunk files");
        }

        return 0;
    }


    else if (strcmp(argv[2],"-r") == 0)
    {
        printf("> Partition 3\n");
        printf("final chunks: %d\n", chunks);

        int forkNum = hasChunks ? number_of_files : number_of_files - 9;

        printf("> FORKNUM: %d\n", forkNum);

        printf("> forkNum: %d\n", forkNum);

        pid_t pid[forkNum];
        for (int i = 0 ; i < forkNum ; i++)
        {
            pid[i] = fork();

            if (pid[i] < 0){
                printf("[-]Files does not splitted correctly.\n");
                return (1);
            }
            else if (pid[i] == 0)
            {
                source_path = argv[3 + i];
                printf("> pid: %p > src: %s\n", &pid[i], source_path);
                char* filename = GetFileName(source_path);
                FileSplitter(source_path, chunks, filename);
                send_file(sockfd, source_path, chunks);
                
                char command[255];
                sprintf(command, "rm %s_*", filename);
                system(command);
            }
            else
            {
                int status;
                wait(&status);
                
                if ( WIFEXITED(status) && i == 0 && pid[i] > 0)
                {
                    printf("> in GodFather!\n");
                    printf("[+]File data sent successfully.\n");
                    printf("[+]Closing the connection.\n");
                    close(sockfd);
                    printf("[+]Connection closed successfully.\n");
                    return 0;
                }
            }
        }
    }

    else 
    {
        printf("[-]Error in command inputs.\n");
        return 1;
    }

    return 0;
}
