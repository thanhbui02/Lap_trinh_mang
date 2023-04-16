#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[])
{
    // Kiểm tra tham số đầu vào
    if (argc != 3)
    {
        printf("Sử dụng: %s <địa chỉ IP> <cổng>\n", argv[0]);
        exit(1);
    }

    // Tạo socket
    int client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock == -1)
    {
        perror("socket() failed");
        exit(1);
    }

    // Thiết lập địa chỉ và cổng của server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // Kết nối đến server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect() failed");
        exit(1);
    }

    // Nhập dữ liệu từ bàn phím và gửi đến server
    char buffer[BUFFER_SIZE];
    // Nhận phản hồi từ server
    int received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (received == -1)
    {
        perror("recv() failed");
        exit(1);
    }
    buffer[received] = 0;
    printf("Phản hồi từ server: %s\n", buffer);
    while (1)
    {
        printf("Nhập dữ liệu để gửi đến server: ");
        fflush(stdout);
        fflush(stdin);
        memset(buffer,'\0',BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        // Gửi dữ liệu đến server
        int len = strlen(buffer);
        int sent = send(client_sock, buffer, len, 0);
        if (sent != len)
        {
            perror("send() failed");
            exit(1);
        }
    }

    // Đóng kết nối
    close(client_sock);
    return 0;
}
