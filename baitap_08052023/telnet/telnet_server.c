#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
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

int main()
{
    int server_fd, client_fds[MAX_CLIENTS], max_clients = MAX_CLIENTS;
    struct sockaddr_in server_addr, client_addr;
    fd_set read_fds;
    int max_fd, activity, i, valread, sd, new_socket, addrlen;
    char buffer[BUFFER_SIZE];

    // Khởi tạo mảng client_fds và set các giá trị thành 0
    for (i = 0; i < max_clients; i++)
    {
        client_fds[i] = 0;
    }

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ và cổng của server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);

    // Liên kết socket với địa chỉ và cổng
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối đến từ clients
    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(client_addr);

    while (1)
    {
        // Xóa tập hợp read_fds và thêm server socket vào tập hợp này
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

        // Thêm các socket client vào tập hợp read_fds
        for (i = 0; i < max_clients; i++)
        {
            sd = client_fds[i];

            // Nếu socket có giá trị hợp lệ, thêm vào tập hợp read_fds

            if (sd > 0)
            {
                FD_SET(sd, &read_fds);
            }

            // Tìm socket có giá trị lớn nhất để sử dụng cho hàm select()
            if (sd > max_fd)
            {
                max_fd = sd;
            }
        }

        // Sử dụng hàm select() để chờ sự kiện trên các socket
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select failed");
        }

        // Kiểm tra xem có kết nối mới từ client hay không
        if (FD_ISSET(server_fd, &read_fds))
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0)
            {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, IP is : %s, port : %d\n",
                   new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Gửi lời chào mừng đến client
            char *welcome_message = "Welcome to the Telnet server!\nPlease enter your username and password!\n";
            send(new_socket, welcome_message, strlen(welcome_message), 0);

            // Thêm socket mới vào mảng client_fds
            for (i = 0; i < max_clients; i++)
            {
                if (client_fds[i] == 0)
                {
                    client_fds[i] = new_socket;
                    break;
                }
            }
        }

        // Xử lý các sự kiện từ các client đã kết nối
        for (i = 0; i < max_clients; i++)
        {
            sd = client_fds[i];

            if (FD_ISSET(sd, &read_fds))
            {
                // Đọc dữ liệu từ client
                memset(buffer, 0, BUFFER_SIZE);
                valread = recv(sd, buffer, BUFFER_SIZE, 0);
                // Xử lý dữ liệu từ client
                if (valread <= 0)
                {
                    // Đóng kết nối nếu không nhận được dữ liệu từ client
                    getpeername(sd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
                    printf("Host disconnected, ip %s, port %d\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    close(sd);
                    client_fds[i] = 0;
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
                            send(sd, success_message, strlen(success_message), 0);
                        }
                        else
                        {
                            char *error_message = "Invalid username or password\n";
                            send(sd, error_message, strlen(error_message), 0);
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
                            send(sd, result_buffer, strlen(result_buffer), 0);
                            fseek(fp, 0, SEEK_END);
                            long size = ftell(fp);
                            if (size == 0)
                            {
                                char *error_message = "Error executing command\n";
                                send(sd, error_message, strlen(error_message) + 1, 0);
                            }
                            fclose(fp);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
