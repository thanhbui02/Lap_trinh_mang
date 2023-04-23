#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Server nhan file tu client

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

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    char buf[2048];
    char *str;
    int ret;

    while (1)
    {
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        if (ret < sizeof(buf))
        {
            buf[ret] = 0;
        }
        printf("%s\n", buf);

        char *str = strtok(buf, " ");

        int n = atoi(str);
        str = strtok(NULL, " ");

        printf("Tên máy tính %s\n", str);
        printf("Số ổ đĩa : %d\n", n);
        for (int i = 0; i < n; i++)
        {
            str = strtok(NULL, " ");
            printf("\t%s - ", str);
            str = strtok(NULL, " ");
            printf("%sGB\n", str);
        }
    }
    close(client);
    close(listener);
}