#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define NUM_THREADS 4  // Số lượng luồng (threads)

void *handle_client(void *arg) {
    int client = *(int *)arg;

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

    pthread_exit(NULL);
}

int main() {
    int listener;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len;

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

    pthread_t threads[NUM_THREADS];
    int thread_index = 0;

    // Tạo luồng (threads)
    while (1) {
        // Chờ kết nối mới
        int client = accept(listener, (struct sockaddr *)&client_addr, &client_len);
        printf("New client connected: %d\n", client);

        // Tạo luồng (threads) mới để xử lý client
        if (pthread_create(&threads[thread_index], NULL, handle_client, &client) != 0) {
            perror("Error creating thread");
            close(client);
            continue;
        }

        // Di chuyển vị trí index của luồng (threads)
        thread_index = (thread_index + 1) % NUM_THREADS;
    }

    // Đóng socket lắng nghe
    close(listener);

    return 0;
}
