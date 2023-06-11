#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Kiểm tra thông tin đăng nhập
int check_credentials(char *username, char *password)
{
    FILE *fp;
    char line[BUFFER_SIZE];
    char *token;
    fp = fopen("credentials.txt", "r");
    if (fp == NULL)
    {
        perror("Error opening credentials file");
        return 0;
    }

    // Đọc từng dòng trong file văn bản
    while (fgets(line, sizeof(line), fp))
    {
        // Tách dòng thành tên người dùng và mật khẩu
        token = strtok(line, " ");
        if (token != NULL && strcmp(token, username) == 0)
        {
            token = strtok(NULL, " \n");
            if (token != NULL && strcmp(token, password) == 0)
            {
                fclose(fp);
                return 1; // Đúng thông tin đăng nhập
            }
        }
    }

    fclose(fp);
    return 0; // Sai thông tin đăng nhập
}

// Luồng xử lý kết nối từ client
void *client_thread(void *arg)
{
    int new_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    // Gửi lời chào mừng đến client
    char *welcome_message = "Welcome to the Telnet server!\nPlease enter your username and password!\n";
    send(new_socket, welcome_message, strlen(welcome_message), 0);

    // Xử lý các sự kiện từ client đã kết nối
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(new_socket, buffer, BUFFER_SIZE, 0);
        // Xử lý dữ liệu từ client
        if (valread <= 0)
        {
            // Đóng kết nối nếu không nhận được dữ liệu từ client
            struct sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);
            getpeername(new_socket, (struct sockaddr *)&addr, &addr_len);
            printf("Host disconnected, ip %s, port %d\n",
                   inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            close(new_socket);
            pthread_exit(NULL);
        }
        else
        {
            // Xử lý thông tin đăng nhập
            if (strncmp(buffer, "userpass:", 9) == 0)
            {
                char *username = strtok(buffer + 9, " ");
                char *password = strtok(NULL, "\n");
                int authenticated = check_credentials(username, password);

                if (authenticated)
                {
                    char *success_message = "Login successful\n";
                    send(new_socket, success_message, strlen(success_message), 0);
                }
                else
                {
                    char *error_message = "Invalid username or password\n";
                    send(new_socket, error_message, strlen(error_message), 0);
                }
            }
            else
            {
                // Thực hiện lệnh từ client
                char command[BUFFER_SIZE];
                snprintf(command, BUFFER_SIZE, "%.900s > out.txt", buffer);
                system(command);

                // Đọc kết quả từ file out.txt
                FILE *fp = fopen("out.txt", "r");

                if (fp != NULL)
                {
                    char result_buffer[BUFFER_SIZE];
                    memset(result_buffer, 0, BUFFER_SIZE);
                    fread(result_buffer, sizeof(char), BUFFER_SIZE - 1, fp);
                    send(new_socket, result_buffer, strlen(result_buffer), 0);
                    fseek(fp, 0, SEEK_END);
                    long size = ftell(fp);
                    if (size == 0)
                    {
                        char *error_message = "Error executing command\n";
                        send(new_socket, error_message, strlen(error_message) + 1, 0);
                    }
                    fclose(fp);
                }
            }
        }
    }
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, client_thread, &client) != 0)
        {
            perror("pthread_create() failed");
            close(client);
            continue;
        }
        
        pthread_detach(thread);
    }

    close(listener);
    return 0;
}
