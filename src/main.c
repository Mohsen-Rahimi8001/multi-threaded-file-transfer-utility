#include "networking.h"

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

    send_file(dest_ip, 8092, source_path);

    return 0;
}