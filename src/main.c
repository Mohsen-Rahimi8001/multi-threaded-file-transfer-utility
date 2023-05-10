#include "networking.h"


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
}



int main(int argc, char* argv[]) {

    char* dest_ip;
    char* source_path;
    long int chunk_size;

    if (argc < 4) {
        printf("[-]Not enough arguments.\n");
        return 1;
    }

    dest_ip = argv[1];
    source_path = argv[2];
    chunk_size = strtol(argv[3], NULL, 10);




    //--> convert FILE to some splited FILEs
    FileSplitter("/home/h00man/comp & Tech/OS/multi-threaded-file-transfer-utility/src/pdfTest.pdf", 10);





    //--> using multi-processing to assign each part to a finction






    //--> sending files in each thread

        // send_file(dest_ip, 8092, source_path);

    return 0;
}