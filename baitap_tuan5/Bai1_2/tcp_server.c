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
    addr.sin_port = htons(8080);

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
    printf("Đang chờ client kết nối!\n");
    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    char buf[2048];
    char *str = "0123456789";
    int count = 0;
    int ret;

    while (1)
    {
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        if (ret <= sizeof(buf))
        {
            buf[ret] = 0;
        }
    }
    char *result = buf;

    while ((result = strstr(result, str)) != NULL)
    {
        {
            count++;
            result = result + strlen(str);
        }
    }
    printf("Số lần xuất hiện của xâu ký tự là : %d\n", count);
    close(client);
    close(listener);
}