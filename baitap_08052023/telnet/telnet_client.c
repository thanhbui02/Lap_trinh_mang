#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main()
{
    int sock;
    char credentials[3000];
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
    char loginSuccess[] = "Login successful\n";
    struct sockaddr_in server_addr;
    char server_ip[] = "127.0.0.1";
    int server_port = 8888;
    char buffer[BUFFER_SIZE];

    // Tạo socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Thiết lập thông tin server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Kết nối tới server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Nhận thông điệp chào mừng từ server
    ssize_t num_bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (num_bytes > 0)
    {
        buffer[num_bytes] = '\0';
        printf("%s", buffer);
    }

    while (1)
    {
        // Gửi thông tin đăng nhập cho server
        memset(username, '\0', BUFFER_SIZE);
        printf("Enter username: ");
        fflush(stdin);
        fgets(username, BUFFER_SIZE, stdin);
        username[strcspn(username, "\n")] = '\0'; // Loại bỏ kí tự xuống dòng

        memset(password, '\0', BUFFER_SIZE);
        printf("Enter password: ");
        fflush(stdin);
        fgets(password, BUFFER_SIZE, stdin);
        password[strcspn(password, "\n")] = '\0'; // Loại bỏ kí tự xuống dòng

        sprintf(credentials, "userpass:%s %s", username, password);

        send(sock, credentials, strlen(credentials), 0);

        // Nhận phản hồi từ server
        char check_credentials[BUFFER_SIZE];
        num_bytes = recv(sock, check_credentials, BUFFER_SIZE - 1, 0);
        if (num_bytes > 0)
        {
            check_credentials[num_bytes] = '\0';
            printf("%s", check_credentials);
        }
        // Gửi và nhận lệnh và kết quả từ server
        if (strcmp(check_credentials, loginSuccess) == 0)
        {
            while (1)
            {
                memset(buffer, '\0', BUFFER_SIZE);
                printf("Enter command (or 'exit' to quit): ");
                fgets(buffer, BUFFER_SIZE, stdin);

                // Loại bỏ kí tự xuống dòng từ chuỗi buffer
                buffer[strcspn(buffer, "\n")] = '\0';

                send(sock, buffer, strlen(buffer), 0);

                if (strcmp(buffer, "exit") == 0)
                {
                    break;
                }

                num_bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
                if (num_bytes > 0)
                {
                    buffer[num_bytes] = '\0';
                    printf("%s", buffer);
                }
            }
            break;
        }
    }

    // Đóng kết nối
    close(sock);

    return 0;
}
