#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 65535

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    // Tạo socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("Failed to create socket");
        return 1;
    }

    // Liên kết socket với cổng chờ
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Failed to bind socket");
        close(sock);
        return 1;
    }

    printf("UDP receiver is running on port %s\n", argv[1]);

    // Nhận dữ liệu
    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);
    char buffer[MAX_BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, MAX_BUFFER_SIZE);

        int bytes_received = recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
        if (bytes_received < 0) {
            perror("Failed to receive data");
            break;
        }

        printf("Received %d bytes from %s:%d\n", bytes_received, inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port));

        // Lấy tên file từ dữ liệu nhận được
        char filename[1024];
        strncpy(filename, buffer, sizeof(filename));

        // Lấy nội dung file từ dữ liệu nhận được
        char *filedata = buffer + strlen(filename) + 1;
        int filedata_size = bytes_received - strlen(filename) - 1;

        // Lưu nội dung file vào file có tên tương ứng
        FILE *fp = fopen(filename, "ab");
        if (fp == NULL) {
            perror("Failed to create file");
            break;
        }

        if (fwrite(filedata, 1, filedata_size, fp) != filedata_size) {
            perror("Failed to write file");
            fclose(fp);
            break;
        }

        fclose(fp);
    }

    close(sock);
    return 0;
}
