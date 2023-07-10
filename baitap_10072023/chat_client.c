#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
//20204851
#define MAX_MESSAGE_LENGTH 200

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Địa chỉ IP của server
    serverAddr.sin_port = htons(8080); // Cổng kết nối tới server

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, addrSize) == 0) {
        printf("Connected to server\n");

        // Gửi yêu cầu JOIN với tên của client
        char joinCommand[30];
        printf("Enter your nickname: ");
        fgets(joinCommand, sizeof(joinCommand), stdin);
        joinCommand[strcspn(joinCommand, "\n")] = '\0';
        send(clientSocket, joinCommand, strlen(joinCommand), 0);

        while (1) {
            char message[MAX_MESSAGE_LENGTH];
            printf("Enter message: ");
            fgets(message, sizeof(message), stdin);
            message[strcspn(message, "\n")] = '\0';
            send(clientSocket, message, strlen(message), 0);

            // Nhận phản hồi từ server
            char response[10];
            recv(clientSocket, response, sizeof(response) - 1, 0);
            response[strlen(response)] = '\0';
            printf("Response from server: %s\n", response);

            // Kiểm tra nếu client gửi lệnh QUIT, kết thúc kết nối
            if (strcmp(message, "QUIT") == 0)
                break;
        }

        printf("Connection closed\n");
    } else {
        printf("Failed to connect to server\n");
    }

    close(clientSocket);

    return 0;
}
