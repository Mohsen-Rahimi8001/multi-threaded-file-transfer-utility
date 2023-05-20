#include "networking.h"


unsigned long int FindSize(char* path)
{
    FILE* file = fopen(path, "rb");  // Open file for binary read

    fseek(file, 0L, SEEK_END);  // Move file pointer to end of file
    long int size = ftell(file);  // Get file size
    fclose(file);  // Close file

    return size;
}



void FileSplitter(char* filePath, int partitions)
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
        sprintf(output_file_name, "output_file_%d", i);
        output_file = fopen(output_file_name, "wb");
        
        if (output_file == NULL) {
            printf("Error creating output file %d.\n", i);
            exit(1);
        }
        
        unsigned long int threashold = CHUNK_SIZE * i + 1;
        size_t readBytes;
        
        while (ftell(input_file) < threashold &&
         (readBytes = fread(buffer, 1, BUF_SIZE, input_file)) > 0) {
            fwrite(buffer, 1, readBytes, output_file);            
        }

        fclose(output_file);
    }

    fclose(input_file); 
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




    //--> convert FILE to some splited FILEs
    FileSplitter(source_path, chunks);


    //--> using multi-processing to assign each part to a finction






    //--> sending files in each thread

    send_file(dest_ip, 10080, source_path, chunks);

    return 0;
}