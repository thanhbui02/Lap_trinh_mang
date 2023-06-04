#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define NUM_CHILDREN 4  // Số lượng tiến trình con

void handle_client(int client) {
    // Nhận dữ liệu từ client và in ra màn hình
    char buf[256];
    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = '\0';
    printf("%s\n", buf);

    // Trả lại kết quả cho client
    char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
    send(client, msg, strlen(msg), 0);

    // Đóng kết nối
    close(client);
}

int main() {
    int listener;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    int i;

    // Khởi tạo socket
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Cấu hình địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    // Gắn địa chỉ server vào socket
    if (bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding");
        exit(1);
    }

    // Lắng nghe kết nối
    if (listen(listener, 5) < 0) {
        perror("Error listening");
        exit(1);
    }

    printf("Server started on port 8080.\n");

    // Tạo tiến trình con
    for (i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Tiến trình con xử lý client
            while (1) {
                // Chờ kết nối mới
                int client = accept(listener, (struct sockaddr *)&client_addr, &client_len);
                printf("New client connected: %d\n", client);

                // Xử lý client
                handle_client(client);
            }
        } else if (pid < 0) {
            perror("Error forking");
            exit(1);
        }
    }

    // Tiến trình cha chờ tiến trình con kết thúc
    for (i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }

    // Đóng socket lắng nghe
    close(listener);

    return 0;
}
