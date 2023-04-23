#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Error!\n");
        return 0;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(9000);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Error!\n");
        return 0;
    }
    printf("Đã kết nối!!\n");
    char computer[200];
    char message[256];
    while (1)
    {
        memset(message, 0, sizeof(message));
        memset(computer, 0, sizeof(computer));
        printf("Nhập vào tên máy tính: ");
        scanf("%s", computer);
        int n;
        printf("Nhập vào số ổ đĩa : ");
        scanf("%d", &n);
        char num[10];
        sprintf(num, "%d", n);
        strncpy(message + strlen(message), num, sizeof(num));
        strncpy(message + strlen(message), " ", 1);
        strncpy(message + strlen(message), computer, sizeof(computer));
        for (int i = 0; i < n; i++)
        {
            char name[10];
            char dungluong[10];

            printf("Nhập vào tên ổ đĩa %d: ", i + 1);
            scanf("%s", name);

            printf("Nhập vào dung lượng ổ đĩa %s: ", name);
            scanf("%s", dungluong);
            strncpy(message + strlen(message), " ", 1);
            strncpy(message + strlen(message), name, sizeof(name));
            strncpy(message + strlen(message), " ", 1);
            strncpy(message + strlen(message), dungluong, sizeof(dungluong));
        }

        send(sock, message, strlen(message), 0);
        printf("\n\n");
    }

    close(sock);
    return 0;
}