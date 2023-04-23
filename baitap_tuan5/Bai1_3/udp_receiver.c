#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    FILE *file = fopen("rec.txt", "w");
    if (file == NULL)
    {
        perror("fopen");
        close(receiver);
        return EXIT_FAILURE;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    char buf[256];
    struct sockaddr_in sender_addr;
    int sender_addr_len = sizeof(sender_addr);

    while (1)
    {
        printf("Đang chờ nhận dữ liệu!\n");
        int ret = recvfrom(receiver, buf, sizeof(buf), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
        if (ret == -1)
        {
            perror("recvfrom");
            fclose(file);
            close(receiver);
            return 0;
        }
        if (ret < sizeof(buf))
            buf[ret] = 0;
        fprintf(file, "%s", buf);

        printf("Đã nhận dữ liệu %d bytes từ %s là: %s và ghi vào file .\n", ret, inet_ntoa(sender_addr.sin_addr), buf);
    }
    fclose(file);
    close(receiver);
}