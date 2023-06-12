#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

void send_file_list(int client_socket, const char *directory_path)
{
    DIR *dir;
    struct dirent *entry;
    char response[BUFFER_SIZE];
    int file_count = 0;

    dir = opendir(directory_path);
    if (dir == NULL)
    {
        sprintf(response, "ERROR No files to download \r\n");
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }

    // Đọc danh sách file trong thư mục
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        { // Chỉ xử lý các file thường (không phải thư mục)
            file_count++;
            snprintf(response, BUFFER_SIZE, "%s\r\n", entry->d_name);
            send(client_socket, response, strlen(response), 0);
        }
    }

    closedir(dir);

    if (file_count == 0)
    {
        sprintf(response, "ERROR No files to download \r\n");
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }

    sprintf(response, "OK %d\r\n\r\n", file_count);
    write(client_socket, response, strlen(response));
}

void send_file(int client_socket, const char *file_name)
{
    FILE *file = fopen(file_name, "rb");
    char response[BUFFER_SIZE];

    if (file == NULL)
    {
        sprintf(response, "ERROR File not found\r\n");
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    sprintf(response, "OK %zu\r\n", file_size);
    write(client_socket, response, strlen(response));

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    {
        write(client_socket, buffer, bytes_read);
    }

    fclose(file);
    close(client_socket);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length;
    pid_t pid;

    const int port = 8888;
    const char *directory_path = ".";

    // Tạo socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    // Thiết lập địa chỉ server
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // Gán địa chỉ server với socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Error binding socket");
        exit(1);
    }

    // Lắng nghe kết nối từ client
    if (listen(server_socket, 5) < 0)
    {
        perror("Error listening on socket");
        exit(1);
    }

    printf("Server started. Listening on port %d\n", port);

    while (1)
    {
        client_address_length = sizeof(client_address);

        // Chấp nhận kết nối từ client
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket < 0)
        {
            perror("Error accepting connection");
            exit(1);
        }

        // Tạo một tiến trình con để xử lý kết nối
        pid = fork();

        if (pid < 0)
        {
            perror("Error creating child process");
            exit(1);
        }

        if (pid == 0)
        {
            // Tiến trình con

            close(server_socket); // Đóng socket lắng nghe trong tiến trình con

            // Gửi danh sách file cho client
            send_file_list(client_socket, directory_path);

            // Đọc tên file từ client
            char file_name[BUFFER_SIZE];
            memset(file_name, 0, BUFFER_SIZE);
            read(client_socket, file_name, BUFFER_SIZE - 1);
            file_name[strcspn(file_name, "\r\n")] = '\0'; // Xóa ký tự xuống dòng

            // Gửi file cho client
            send_file(client_socket, file_name);

            exit(0);
        }
        else
        {
            // Tiến trình cha
            close(client_socket); // Đóng socket kết nối trong tiến trình cha
        }
    }

    return 0;
}
