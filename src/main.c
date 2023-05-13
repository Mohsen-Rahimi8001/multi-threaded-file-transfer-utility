#include "networking.h"


struct send_file_args myArgs;


int FindSize(char* path)
{
    FILE* file = fopen(path, "rb");  // Open file for binary read

    fseek(file, 0L, SEEK_END);  // Move file pointer to end of file
    long int size = ftell(file);  // Get file size
    fclose(file);  // Close file

    return size;
}



void FileSplitter(char* filePath, int partition)
{
    pthread_t threads[partition];
    char** splitedFiles_name = (char**) malloc(partition * sizeof(char*));

    for (int i = 0; i < partition; i++) {
        char temp[50];
        sprintf(temp, "output_file_%d", i + 1);
        splitedFiles_name[i] = (char*) malloc(strlen(temp) + 1);
        strcpy(splitedFiles_name[i], temp);
    }


    FILE* input_file = fopen(filePath, "rb");  // Open input file for binary read
    if (input_file == NULL) {
        printf("Error opening input file.\n");
        exit(1);
    }

    int CHUNK_SIZE = (FindSize(filePath) / partition) + 1 ;  // Chunk size in bytes
    char buffer[CHUNK_SIZE];  // Buffer for reading input file

    int chunk_number = 1;  // Number of current output chunk
    while (!feof(input_file)) {
        FILE* output_file;
        char output_file_name[50];
        sprintf(output_file_name, "output_file_%d", chunk_number);  // Generate output file name
        output_file = fopen(output_file_name, "wb");  // Open output file for binary write
        if (output_file == NULL) {
            printf("Error creating output file %d.\n", chunk_number);
            exit(1);
        }

        int bytes_read = fread(buffer, 1, CHUNK_SIZE, input_file);  // Read a chunk from input file
        if (bytes_read > 0) {
            fwrite(buffer, 1, bytes_read, output_file);  // Write chunk to output file
            chunk_number++;
        }

        fclose(output_file); 
    }

    fclose(input_file); 

    for (int i = 0; i < partition; i++) {
        pthread_create(&threads[i], NULL, send_file, &myArgs);
    }

    for (int i = 0; i < partition; i++) {
        pthread_join(threads[i], NULL);
    }

}



int main(int argc, char* argv[]) {

    char* dest_ip;
    char* source_path;
    int chunks;

    if (argc < 4) {
        printf("[-]Not enough arguments.\n");
        return 1;
    }

    dest_ip = argv[1];
    source_path = argv[2];
    chunks = atoi(argv[3]);

    
//    myArgs.dest_ip = dest_ip;
//    myArgs.port = 8092;
//    myArgs.source_path = source_path;



    // //=============//
    // source_path = "pdfTest.pdf"; 
    // //=============//
    // chunk_size = 7;
    // //=============//


    FILE* selected_file = fopen(source_path, "rb");
    if (selected_file == NULL) {
        char temp[1024];
        strcat(source_path, "/");
        strcat(source_path, getcwd(temp, sizeof(temp)));
    }


    //--> using multi-processing to assign each part to a finction
  
  
    //--> convert FILE to some splited FILEs
    FileSplitter(source_path, chunks);









    //--> sending files in each thread

//    send_file(dest_ip, 10092, source_path, chunks);


    return 0;
}