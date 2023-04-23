#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sender == -1)
    {
        perror("socket");
        exit(1);
    }


    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(argv[2]);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    char buf[256];
    FILE *f = fopen(argv[3], "rb");
    int ret = fread(buf, 1, sizeof(buf), f);
    buf[ret] = 0;
    sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&addr,
           sizeof(addr));
    printf("Đã gửi dữ liệu từ file %s qua server. \n", argv[3]);
    close(sender);
}