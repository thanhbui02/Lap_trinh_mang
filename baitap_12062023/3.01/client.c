#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
char response[BUFFER_SIZE];
void download_file(int server_socket, const char *file_name)
{

    FILE *file;
    off_t file_size;
    off_t total_bytes_received = 0;

    // Gửi tên file cho server
    snprintf(response, BUFFER_SIZE, "%s\r\n", file_name);
    write(server_socket, response, strlen(response));

    // Đọc phản hồi từ server
    memset(response, 0, BUFFER_SIZE);
    recv(server_socket, response, BUFFER_SIZE - 1, 0);

    if (strncmp(response, "OK", 2) != 0)
    {
        printf("Server error: %s\n", response);
        return;
    }

    sscanf(response, "OK %lld", (long long *)&file_size);

    if (file_size == 0)
    {
        printf("File not found on server\n");
        return;
    }

    // Tạo file để ghi nội dung nhận được từ server
    file = fopen(file_name, "w");
    if (file == NULL)
    {
        printf("Error creating file\n");
        return;
    }

    // Nhận nội dung file từ server
    while (total_bytes_received < file_size)
    {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received = read(server_socket, buffer, BUFFER_SIZE);
        fwrite(buffer, 1, bytes_received, file);
        total_bytes_received += bytes_received;
    }

    fclose(file);
    printf("File downloaded successfully\n");
}

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8888);
    // Kết nối tới server
    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("connect() failed");
        return 1;
    }

    char file_name[BUFFER_SIZE];
    recv(client, response, BUFFER_SIZE - 1, 0);
    printf("Enter the file name to download: ");
    fgets(file_name, BUFFER_SIZE, stdin);
    file_name[strcspn(file_name, "\n")] = '\0'; // Remove newline character

    // Tải về file từ server
    download_file(client, file_name);

    // Đóng kết nối
    close(client);

    return 0;
}
