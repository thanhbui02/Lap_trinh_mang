#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <ip> <port> <file>\n", argv[0]);
        exit(1);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *file_path = argv[3];

    // Open file
    int file_fd = open(file_path, O_RDONLY);
    if (file_fd == -1) {
        perror("open");
        exit(1);
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    // Set destination address
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(ip);
    dest_addr.sin_port = htons(port);

    // Send file
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    ssize_t bytes_sent;
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_sent = sendto(sock, buffer, bytes_read, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (bytes_sent == -1) {
            perror("sendto");
            exit(1);
        }
        usleep(1000);
    }

    // Close file and socket
    close(file_fd);
    close(sock);

    return 0;
}
