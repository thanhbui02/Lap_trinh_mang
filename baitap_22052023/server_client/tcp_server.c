#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int serverSocket, clientSockets[MAX_CLIENTS], maxClients = MAX_CLIENTS;
    struct sockaddr_in serverAddress, clientAddress;
    fd_set readfds;
    int activity, i, valread, sd, max_sd;
    int addrlen = sizeof(clientAddress);
    char buffer[BUFFER_SIZE];

    // Tạo socket máy chủ
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Không thể tạo socket máy chủ");
        exit(EXIT_FAILURE);
    }

    // Cấu hình địa chỉ máy chủ
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8888);

    // Ràng buộc socket máy chủ với địa chỉ và cổng
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Ràng buộc socket thất bại");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ client
    if (listen(serverSocket, maxClients) < 0) {
        perror("Lỗi lắng nghe");
        exit(EXIT_FAILURE);
    }

    printf("Máy chủ đang lắng nghe kết nối...\n");

    // Chấp nhận kết nối từ client và thêm vào mảng clientSockets
    for (i = 0; i < maxClients; i++) {
        clientSockets[i] = 0;
    }

    while (1) {
        // Xóa tập hợp file descriptor và thêm serverSocket vào tập hợp
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        max_sd = serverSocket;

        // Thêm các socket client vào tập hợp
        for (i = 0; i < maxClients; i++) {
            sd = clientSockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Sử dụng select để kiểm tra xem có hoạt động trên bất kỳ socket nào
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            printf("Lỗi trong select");
        }

        // Kiểm tra kết nối đến socket máy chủ (serverSocket)
        if (FD_ISSET(serverSocket, &readfds)) {
            int newSocket;
            if ((newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t *)&addrlen)) < 0) {
                perror("Lỗi chấp nhận kết nối");
                exit(EXIT_FAILURE);
            }

            printf("Kết nối mới, socket fd: %d, IP: %s, cổng: %d\n", newSocket, inet_ntoa(clientAddress.sin_addr),
                   ntohs(clientAddress.sin_port));

            // Gửi thông báo đến client về số lượng client hiện tại đang kết nối
            char message[50];
            sprintf(message, "Xin chào. Hiện có %d clients đang kết nối.\n", i + 1);
            send(newSocket, message, strlen(message), 0);

            // Thêm socket client mới vào mảng clientSockets
            for (i = 0; i < maxClients; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = newSocket;
                    break;
                }
            }
        }

        // Kiểm tra hoạt động trên các socket client
        for (i = 0; i < maxClients; i++) {
            sd = clientSockets[i];
            if (FD_ISSET(sd, &readfds)) {
                // Đọc dữ liệu từ client
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // Mất kết nối hoặc client gửi "exit"
                    getpeername(sd, (struct sockaddr *)&clientAddress, (socklen_t *)&addrlen);
                    printf("Client ngắt kết nối, IP: %s, cổng: %d\n", inet_ntoa(clientAddress.sin_addr),
                           ntohs(clientAddress.sin_port));

                    // Đóng socket và xóa khỏi mảng clientSockets
                    close(sd);
                    clientSockets[i] = 0;
                } else {
                    // Xử lý yêu cầu từ client
                    buffer[valread] = '\0';

                    // Chuẩn hóa xâu ký tự
                    for (int j = 0; j < valread; j++) {
                        if (buffer[j] == '\n' || buffer[j] == '\r') {
                            buffer[j] = ' ';
                        }
                    }

                    // Viết hoa chữ cái đầu từ và xóa khoảng trắng thừa
                    char *token = strtok(buffer, " ");
                    while (token != NULL) {
                        if (isalpha(token[0])) {
                            token[0] = toupper(token[0]);
                        }
                        printf("%s ", token);
                        token = strtok(NULL, " ");
                    }
                    printf("\n");

                    // Gửi kết quả trả về cho client
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    return 0;
}
